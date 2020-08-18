-- Autor: Martin Fekete xfeket00

library IEEE;
use IEEE.std_logic_1164.all;
use IEEE.std_logic_arith.all;
use IEEE.std_logic_unsigned.all;

entity ledc8x8 is
    port (
        SMCLK : in std_logic;
        RESET : in std_logic;
        ROW   : out std_logic_vector (0 to 7);
        LED   : out std_logic_vector (0 to 7)
    );
end ledc8x8;

architecture main of ledc8x8 is
signal leds: std_logic_vector(7 downto 0);
signal rows: std_logic_vector(7 downto 0);

signal ce: std_logic := '0';
signal ce_cnt: std_logic_vector(12 downto 0) := (others => '0'); -- counter periody riadku
signal ce_cnt2: std_logic_vector(22 downto 0) := (others => '0'); -- counter na vypnutie/zapnutie displaya

signal state: std_logic := '1';

begin

    -- delicka na znizenie frekvencie
    ce_gen: process(SMCLK, RESET, ce)
    begin
        if RESET = '1' then
            ce_cnt <= (others => '0');
        elsif SMCLK'event and SMCLK = '1' then
            ce_cnt <= ce_cnt + 1;
            if ce_cnt2 < "11111111111111111111111" then
                ce_cnt2 <= ce_cnt2 + 1;
            end if;
        end if;
    end process ce_gen;
    ce <= '1' when ce_cnt = "111000010000" else '0'; -- zmena riadku - SMCLK/256
    state <= '0' when ce_cnt2 >= "1110000100000000000000" and ce_cnt2 <= "11100001000000000000000" else '1'; -- zmena stavu vypnuty/zapnuty

    -- rotacny register
    rotation: process(SMCLK, RESET, ce)
    begin
        if RESET = '1' then
            rows <= "10000000";
        elsif SMCLK'event and SMCLK = '1' then
            if ce = '1' then
                rows <= rows(0) & rows(7 downto 1);
            end if;
        end if;
    end process rotation;

    decoder: process(rows, state)
    begin
        if state = '1' then
        case rows is
            when "10000000" => leds <= "01110111";
            when "01000000" => leds <= "00100111";
            when "00100000" => leds <= "01010111";
            when "00010000" => leds <= "01110000";
            when "00001000" => leds <= "01110111";
            when "00000100" => leds <= "11110001";
            when "00000010" => leds <= "11110111";
            when "00000001" => leds <= "11110111";
            when others     => leds <= "11111111";
        end case;
        elsif state = '0' then
           leds <= "11111111";
        end if;
    end process decoder;

    LED <= leds;
    ROW <= rows;

end main;




-- ISID: 75579
