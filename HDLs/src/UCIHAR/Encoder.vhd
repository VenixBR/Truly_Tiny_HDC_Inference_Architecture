library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.numeric_std.ALL;

entity Encoder is
    generic (
        PARALLEL            : integer := 4;
        FEATURE_WIDTH       : integer := 8;
        INDEX_WIDTH         : integer := 16; 
        DIMENSIONS          : integer := 8192;
        EFFECTIVE_INDEXES   : integer := 4;
        MAX_X               : integer := 28;
        MAX_Y               : integer := 28;
        INDEXES_IMG         : string := ""
    );
    port (
        clk     : in std_logic;
        rst     : in std_logic;
        start   : in std_logic;
        feature : in std_logic_vector(FEATURE_WIDTH - 1 downto 0);
        address : out std_logic_vector(9 downto 0);
        bits    : out std_logic_vector(PARALLEL - 1 downto 0);
        bits_av : out std_logic;
        halt    : in std_logic; -- Back pressure control
        done    : out std_logic        
    );
end Encoder;

architecture Behavioral of Encoder is   
    
    constant ADDR_WIDTH: integer := 12;
    
    type State is (RESET, INIT_SAMPLE_MEM_ADDR, READ_INDEXES, SAMPLE_MEM_ADDR, SUM, BITS_AVAILABLE, FINISH);
    signal currentState : State;
    
    signal x: UNSIGNED(9 downto 0);
    signal data_av : std_logic;
            
    signal bit_enc_rst: std_logic;
    
    
    -- Memories
    signal samples_addr : UNSIGNED(9 downto 0);
    
    signal indexes_addr: UNSIGNED(ADDR_WIDTH - 1 downto 0);
    
    type IndexesArray is array(natural range <>) of std_logic_vector(INDEX_WIDTH - 1 downto 0);
    signal idxs : IndexesArray(PARALLEL - 1 downto 0);
    signal idx : std_logic_vector(INDEX_WIDTH - 1 downto 0);
    signal i: integer; 
    
    
    constant SAMPLE_SIZE    : integer := MAX_X;
    
    
begin

    address <= STD_LOGIC_VECTOR(samples_addr); 
        
    INDEXES: entity work.Memory(BlockRAM)
        generic map (
            imageFileName   => INDEXES_IMG,         
            DATA_WIDTH      => INDEX_WIDTH,
            ADDR_WIDTH      => ADDR_WIDTH
        )
        port map (
            clock           => clk,
            wr              => '0',
            write_address   => (others=>'0'),
            read_address    => STD_LOGIC_VECTOR(indexes_addr),
            data_i          => (others=>'0'),        
            data_o          => idx
        );
    
    PARALLEL_BIT_ENCODERS: for i in 0 to PARALLEL - 1 generate    
        BIT_ENCODER: entity work.BitEncoder(Behavioral) 
            generic map (
                INDEX_WIDTH => INDEX_WIDTH,
                DIMENSIONS  => DIMENSIONS,
                THRESHOLD   => SAMPLE_SIZE / 2
            )
            port map (
                clk     => clk,
                rst     => bit_enc_rst,
                idx     => idxs(i),
                data    => feature,
                data_av => data_av,
                x       => STD_LOGIC_VECTOR(x),
                b       => bits(i)
            );
            
    end generate PARALLEL_BIT_ENCODERS;
    
 
    bit_enc_rst <= '1' when currentState = INIT_SAMPLE_MEM_ADDR else '0';
    
    data_av <= '1' when currentState = SUM else '0';
    
    bits_av <= '1' when currentState = BITS_AVAILABLE else '0';
    
    done <= '1' when currentState = FINISH else '0';
       
    process(clk, rst)
    begin
        if rst = '1' then
            
            currentState <= RESET;
        
        elsif rising_edge(clk) then
            case currentState is 
                when RESET =>                    
                    indexes_addr <= (others=>'0'); 
                    i <= 0;                   
                    
                    if start = '1' then
                        currentState <= INIT_SAMPLE_MEM_ADDR;
                    end if;
                    
                when INIT_SAMPLE_MEM_ADDR =>
                    x <= (others=>'0');
                    samples_addr <= (others=>'0');
                    indexes_addr <= indexes_addr + 1;
                    currentState <= READ_INDEXES;
                    
                when READ_INDEXES =>                    
                    idxs(i) <= idx;
                    
                    if i < PARALLEL - 1 and indexes_addr < EFFECTIVE_INDEXES then                        
                        i <= i + 1;
                        indexes_addr <= indexes_addr + 1; 
                    else       
                        i <= 0;
                        currentState <= SAMPLE_MEM_ADDR;
                    end if;
                    
                when SAMPLE_MEM_ADDR =>
                    samples_addr <= samples_addr + 1;
                    currentState <= SUM;
                    
                when SUM =>           
                    if samples_addr < SAMPLE_SIZE then
                        samples_addr <= samples_addr + 1;
                    else
                        currentState <= BITS_AVAILABLE;
                    end if;
                    
                    if x < MAX_X - 1 then
                        x <= x + 1;
                    else
                        x <= (others=>'0');
                    end if; 
                    
                when BITS_AVAILABLE =>
                    if halt = '0' then
                        if indexes_addr >= EFFECTIVE_INDEXES  then
                            currentState <= FINISH;
                        else
                            currentState <= INIT_SAMPLE_MEM_ADDR;
                        end if;
                    end if;
                    
                when FINISH =>
                    currentState <= RESET;
                                        
                
            end case;
        end if;  
    end process;

end Behavioral;