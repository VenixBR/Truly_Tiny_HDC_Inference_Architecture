library IEEE;
use IEEE.std_logic_1164.ALL;
use IEEE.numeric_std.ALL;

entity HyperInference_tb is
end HyperInference_tb;

architecture Behavioral of HyperInference_tb is

    constant DIMENSIONS         : integer := 8192;
    constant CLASSES            : integer := 6;
    constant PARALLEL           : integer := 256; 
    constant COUNTER_ADDERS     : integer := 3;
    
    -- Memory image files
    constant SAMPLE_IMG         : string := "UCIHAR_sample.txt";
    constant CLASSES_IMG        : string := "UCIHAR_hvs.txt";
    constant INDEXES_IMG        : string := "UCIHAR_idxs.txt";
    
    -- Memories constants
    constant SAMPLE_ADDR_WIDTH  : integer := 10;
    constant SAMPLE_DATA_WIDTH  : integer := 8;   
    
    constant CLASS_ADDR_WIDTH   : integer := 12;
    constant CLASS_DATA_WIDTH   : integer := CLASSES; 
       
    constant EFFECTIVE_INDEXES  : integer := 3200;        
    
    signal samples_addr : std_logic_vector(SAMPLE_ADDR_WIDTH - 1 downto 0);
    signal feature      : std_logic_vector(SAMPLE_DATA_WIDTH - 1 downto 0);    
       
    signal done: std_logic;    
    
    signal clk      : std_logic := '0';
    signal rst      : std_logic;
    signal start    : std_logic;  
      
begin

    --clk <= not clk after 2.5 ns;  -- 5ns = 200MHz
    --clk <= not clk after 2.75 ns; -- 5.5ns = 181.81MHz
    --clk <= not clk after 3 ns;      -- 6ns = 166,66MHz
    --clk <= not clk after 3.5 ns;    -- 7ns = 142.85MHz
    --clk <= not clk after 3.25 ns;    -- 7.5ns = 133.335MHz
    clk <= not clk after 4 ns;    -- 8ns = 125MHz
    
    rst <= '1', '0' after 5 ns;
            
    HYPER_INFERENCE: entity work.HyperInference(behavioral)
        generic map (
            SAMPLE_ADDR_WIDTH   => SAMPLE_ADDR_WIDTH,
            SAMPLE_DATA_WIDTH   => SAMPLE_DATA_WIDTH,
            CLASS_ADDR_WIDTH    => CLASS_ADDR_WIDTH,
            CLASS_DATA_WIDTH    => CLASS_DATA_WIDTH,
            COUNTER_ADDERS      => COUNTER_ADDERS,
            PARALLEL            => PARALLEL,
            DIMENSIONS          => DIMENSIONS,
            EFFECTIVE_INDEXES   => EFFECTIVE_INDEXES,
            CLASSES             => CLASSES,
            INDEXES_IMG         => INDEXES_IMG,
            CLASSES_IMG         => CLASSES_IMG
        )
        port map (
            clk             => clk,
            rst             => rst,
            start           => start,
            samples_addr    => samples_addr,
            feature         => feature,            
            done            => done           
        );
        
        
    SAMPLE: entity work.Memory(BlockRAM)
        generic map (
            imageFileName   => SAMPLE_IMG,         
            DATA_WIDTH      => SAMPLE_DATA_WIDTH,
            ADDR_WIDTH      => SAMPLE_ADDR_WIDTH
        )
        port map (
            clock           => clk,
            wr              => '0',
            write_address   => (others=>'0'),
            read_address    => STD_LOGIC_VECTOR(samples_addr),
            data_i          => (others=>'0'),        
            data_o          => feature
        );
        
   
    process
    begin
        start <= '0';
        
        wait until rising_edge(clk);
        wait until rising_edge(clk);
        start <= '1';
        
        wait until rising_edge(clk);
        start <= '0';
        
        wait;
    end process;

end Behavioral;
