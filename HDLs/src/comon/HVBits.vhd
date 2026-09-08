----------------------------------------------------------------------------------
-- Company: 
-- Engineer: 
-- 
-- Create Date: 09/22/2025 10:17:18 AM
-- Design Name: 
-- Module Name: HVBits - Behavioral
-- Project Name: 
-- Target Devices: 
-- Tool Versions: 
-- Description: 
-- 
-- Dependencies: 
-- 
-- Revision:
-- Revision 0.01 - File Created
-- Additional Comments:
-- 
----------------------------------------------------------------------------------


library IEEE;
use IEEE.STD_LOGIC_1164.ALL;
use IEEE.NUMERIC_STD.ALL;


entity HVBits is
    port (         
        t       : in std_logic_vector(15 downto 0);
        o       : out std_logic
    );
end HVBits;


architecture MNIST_seed1 of HVBits is

    signal rt: UNSIGNED(15 downto 0);  
    signal rotate: integer;   
    
begin
    
    rotate <= TO_INTEGER(UNSIGNED(t(3 downto 0)));

    rt <= ROTATE_LEFT(UNSIGNED(t), rotate);
   
    -- Python indexes:  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
    -- VHDL indexes:    15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
   
    o <= rt(4) xor rt(5) xor rt(6) xor rt(7);
    
end MNIST_seed1;


architecture MNIST_seed2 of HVBits is

    signal rt: UNSIGNED(15 downto 0);  
    signal rotate: integer;
    
begin
    
    rotate <= TO_INTEGER(UNSIGNED(t(3 downto 0)));

    rt <= ROTATE_LEFT(UNSIGNED(t), rotate);
    
    -- Python indexes:  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
    -- VHDL indexes:    15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
                 
    o <=  rt(8) xor rt(9) xor rt(10) xor rt(11) xor rt(12) xor rt(13) xor rt(14) xor rt(15);
   
end MNIST_seed2;


architecture MNIST_seed3 of HVBits is

    signal rt: UNSIGNED(15 downto 0);  
    signal rotate: integer;
    
begin
    
    rotate <= TO_INTEGER(UNSIGNED(t(3 downto 0)));

    rt <= ROTATE_LEFT(UNSIGNED(t), rotate);
    
    -- Python indexes:  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
    -- VHDL indexes:    15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
  
    o <= rt(2) xor rt(3) xor rt(4) xor rt(5) xor rt(6) xor rt(7) xor rt(12) xor rt(13);
    
end MNIST_seed3;



architecture ISOLET_seed1 of HVBits is

    signal rt: UNSIGNED(15 downto 0);  
    signal rotate: integer;
    
begin
    
    rotate <= TO_INTEGER(UNSIGNED(t(3 downto 0)));

    rt <= ROTATE_LEFT(UNSIGNED(t), rotate);
    
    -- Python indexes:  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
    -- VHDL indexes:    15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
  
    o <= rt(2) xor rt(3) xor rt(6) xor rt(7) xor rt(8) xor rt(9) xor rt(10) xor rt(11) xor rt(12) xor rt(13) xor rt(14) xor rt(15);
    
end ISOLET_seed1;

architecture ISOLET_seed2 of HVBits is

    signal rt: UNSIGNED(15 downto 0);  
    signal rotate: integer;
    
begin
    
    rotate <= TO_INTEGER(UNSIGNED(t(3 downto 0)));

    rt <= ROTATE_LEFT(UNSIGNED(t), rotate);
    
    -- Python indexes:  00 01 02 03 04 05 06 07 08 09 10 11 12 13 14 15
    -- VHDL indexes:    15 14 13 12 11 10 09 08 07 06 05 04 03 02 01 00
  
    o <= not (rt(2) xor rt(3) xor rt(6) xor rt(7) xor rt(8) xor rt(9) xor rt(10) xor rt(11) xor rt(12) xor rt(13) xor rt(14) xor rt(15));
    
end ISOLET_seed2;






