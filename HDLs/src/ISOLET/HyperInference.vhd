library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.numeric_std.ALL;

entity HyperInference is
    generic (
        SAMPLE_ADDR_WIDTH   : integer := 10;
        SAMPLE_DATA_WIDTH   : integer := 8;
        CLASS_DATA_WIDTH    : integer := 26;
        CLASS_ADDR_WIDTH    : integer := 12;
        PARALLEL            : integer := 32;
        COUNTER_ADDERS      : integer := 2;
        DIMENSIONS          : integer := 8192;
        EFFECTIVE_INDEXES   : integer := 8192;
        CLASSES             : integer := 26;
        INDEXES_IMG         : string := "ISOLET_idxs.txt";
        CLASSES_IMG         : string := "ISOLET_hvs.txt"      
    );
    port (
        clk             : in std_logic;
        rst             : in std_logic;
        start           : in std_logic;
        done            : out std_logic;
        class           : out std_logic_vector(4 downto 0);
        samples_addr    : out std_logic_vector(SAMPLE_ADDR_WIDTH - 1 downto 0);
        feature         : in std_logic_vector(SAMPLE_DATA_WIDTH - 1 downto 0)
    );
end HyperInference;

architecture Behavioral of HyperInference is
       
    signal class_addr   : UNSIGNED(CLASS_ADDR_WIDTH - 1 downto 0);
    signal class_bits   : std_logic_vector(CLASS_DATA_WIDTH - 1 downto 0);      
    
    type CountersArray is array(natural range <>) of UNSIGNED(11 downto 0);
    signal counters : CountersArray(0 to CLASSES - 1);
    signal smaller  : UNSIGNED(11 downto 0);
    
    signal bits_av, encoder_done, done_reg: std_logic;
    signal bits, encoded_bits: std_logic_vector(PARALLEL - 1 downto 0);
    signal counting: std_logic;
    
    type State is (INIT, WAITING_BITS, HAMMING, READ_CLASS_HVS_BITS, CLASSIFICATION, FINISHED);
    signal currentState : State;
    
    signal i            : integer;
    signal count_bits   : integer;
          
begin
    HV_ENCODER: entity work.Encoder(Behavioral)
        generic map (
            PARALLEL            => PARALLEL,
            FEATURE_WIDTH       => SAMPLE_DATA_WIDTH,
            INDEX_WIDTH         => 13, 
            DIMENSIONS          => DIMENSIONS,
            EFFECTIVE_INDEXES   => EFFECTIVE_INDEXES,
            MAX_X               => 617,
            INDEXES_IMG         => INDEXES_IMG
        )
        port map (
            clk         => clk,
            rst         => rst,
            start       => start,
            address     => samples_addr,
            feature     => feature,
            bits_av     => bits_av,
            bits        => bits,
            halt        => counting,
            done        => encoder_done           
        );
               
    CLASS_HVS: entity work.Memory(BlockRAM)
        generic map (
            imageFileName   => CLASSES_IMG,         
            DATA_WIDTH      => CLASS_DATA_WIDTH,
            ADDR_WIDTH      => CLASS_ADDR_WIDTH
        )
        port map (
            clock           => clk,
            wr              => '0',
            write_address   => (others=>'0'),
            read_address    => std_logic_vector(class_addr),
            data_i          => (others=>'0'),        
            data_o          => class_bits
        );
        
    -- Backpressure
    -- Used to halt encoder when it is not ready to compute hamming distance (currentState is not WAITING_BITS)
    counting <= '1' when currentState = HAMMING or currentState = READ_CLASS_HVS_BITS else '0';
    
    done <= '1' when currentState = FINISHED else '0';
            
    process(clk, rst)
    begin
        if rst = '1' then
            currentState <= INIT;
            
        elsif rising_edge(clk) then
            case currentState is
                when INIT =>
                    class_addr <= (others=>'0');
                    currentState <= WAITING_BITS;
                    i <= 0;                    
                    
                when WAITING_BITS =>                    
                    encoded_bits <= bits;
                    count_bits <= 0;
                    
                    if bits_av = '1' then                                                
                        currentState <= HAMMING;                       
                    
                    elsif done_reg = '1' then
                        smaller <= counters(0);
                        class <= STD_LOGIC_VECTOR(TO_UNSIGNED(0, class'length));
                        currentState <= CLASSIFICATION;
                    end if;
                    
                when HAMMING =>
                    i <= i + COUNTER_ADDERS;
                        
                    if i + COUNTER_ADDERS >= CLASSES then
                        i <= 0;
                        class_addr <= class_addr + 1;
                        count_bits <= count_bits + 1;                        
                        
                        if count_bits = PARALLEL - 1 then
                            currentState <= WAITING_BITS;
                        else
                            currentState <= READ_CLASS_HVS_BITS;
                        end if;
                    end if;                     
                    
                when READ_CLASS_HVS_BITS =>
                    currentState <= HAMMING;
                    
                when CLASSIFICATION =>
                    if counters(i) < smaller then
                        smaller <= counters(i);
                        class <= STD_LOGIC_VECTOR(TO_UNSIGNED(i, class'length));
                    end if;
                    
                    i <= i + 1;
                    
                    if i = CLASSES - 1 then
                        currentState <= FINISHED;
                    end if;
                    
                when FINISHED =>
                    currentState <= WAITING_BITS; 
                    
                when others =>
            end case;     
        end if;
    end process;
    
    -- Store the done signal generated by Encoder.
    -- This signal can be missed when it arrives and this is not ready to catch it (currentState is not WAITING_BITS) 
    process(clk, rst)
    begin
        if rst = '1' then
            done_reg <= '0';
            
        elsif rising_edge(clk) then
            if encoder_done = '1' then
                done_reg <= '1';
            end if;
            
            if currentState = CLASSIFICATION then
                done_reg <= '0';
            end if;
            
        end if;
    end process;

    
    -- Adders used to compute the hamming distance
    ADDERS: for c in 0 to COUNTER_ADDERS - 1 generate
        process(clk, rst)
        begin
            if rst = '1' then
                counters <= (others=>(others=>'0'));
            elsif rising_edge(clk) then
                if currentState = HAMMING then
                
                    for j in 0 to COUNTER_ADDERS - 1 loop
                        
                        if (i + j) < CLASSES then
                            if encoded_bits(count_bits) = class_bits(i + j) then
                                counters(i + j) <= counters(i + j) + 1;                     
                            end if;
                        end if;
                    end loop;
                
                end if;
            end if;
        end process;
    
    end generate;    
  

end Behavioral;
