-- cpu.vhd: Simple 8-bit CPU (BrainF*ck interpreter)
-- Copyright (C) 2019 Brno University of Technology,
--                    Faculty of Information Technology
-- Author(s): Martin Fekete <xfeket00>
--

library ieee;
use ieee.std_logic_1164.all;
use ieee.std_logic_arith.all;
use ieee.std_logic_unsigned.all;

-- ----------------------------------------------------------------------------
--                        Entity declaration
-- ----------------------------------------------------------------------------
entity cpu is
 port (
   CLK   : in std_logic;  -- hodinovy signal
   RESET : in std_logic;  -- asynchronni reset procesoru
   EN    : in std_logic;  -- povoleni cinnosti procesoru
 
   -- synchronni pamet RAM
   DATA_ADDR  : out std_logic_vector(12 downto 0); -- adresa do pameti
   DATA_WDATA : out std_logic_vector(7 downto 0); -- mem[DATA_ADDR] <- DATA_WDATA pokud DATA_EN='1'
   DATA_RDATA : in std_logic_vector(7 downto 0);  -- DATA_RDATA <- ram[DATA_ADDR] pokud DATA_EN='1'
   DATA_RDWR  : out std_logic;                    -- cteni (0) / zapis (1)
   DATA_EN    : out std_logic;                    -- povoleni cinnosti
   
   -- vstupni port
   IN_DATA   : in std_logic_vector(7 downto 0);   -- IN_DATA <- stav klavesnice pokud IN_VLD='1' a IN_REQ='1'
   IN_VLD    : in std_logic;                      -- data platna
   IN_REQ    : out std_logic;                     -- pozadavek na vstup data
   
   -- vystupni port
   OUT_DATA : out  std_logic_vector(7 downto 0);  -- zapisovana data
   OUT_BUSY : in std_logic;                       -- LCD je zaneprazdnen (1), nelze zapisovat
   OUT_WE   : out std_logic                       -- LCD <- OUT_DATA pokud OUT_WE='1' a OUT_BUSY='0'
 );
end cpu;


-- ----------------------------------------------------------------------------
--                      Architecture declaration
-- ----------------------------------------------------------------------------
architecture behavioral of cpu is

    -- zde dopiste potrebne deklarace signalu

    -- signaly pre PC
    signal pc_reg: std_logic_vector(12 downto 0);
    signal pc_dec: std_logic;
    signal pc_inc: std_logic;
    signal pc_set: std_logic;

    -- signaly pre PTR
    signal ptr_reg: std_logic_vector(12 downto 0);
    signal ptr_dec: std_logic;
    signal ptr_inc: std_logic;
    signal ptr_set: std_logic;

    -- signaly pre CNT
    signal cnt_reg: std_logic_vector(7 downto 0);
    signal cnt_dec: std_logic;
    signal cnt_inc: std_logic;
    signal cnt_set: std_logic;
    signal cnt_set_one: std_logic;

    -- signaly sel pri MX 1-3
    signal sel1: std_logic;                    -- 0 pre adresovanie programu, 1 pre adresovanie dat
    signal sel2: std_logic;                    -- tmp alebo ptr
    signal sel3: std_logic_vector(1 downto 0); -- data_rdata

    -- vystup MX2
    signal mx2_out: std_logic_vector(12 downto 0);

    -- stavy konecneho automatu
    type fsm_state is
        (
            s_start,             -- zaciatocny stav
            s_instr_fetch,       -- nacitanie instrukcie
            s_instr_decode,      -- dekodovanie instrukcie
            s_inc_ptr,           -- > inkrementacia ukazovatela
            s_dec_ptr,           -- < dekrementacia ukazovatela
            s_inc1, s_inc2,      -- + inkrementacia hodnoty aktualnej bunky
            s_dec1, s_dec2,      -- - dekrementacia hodnoty aktualnej bunky
            s_print1, s_print2,  -- . vypis hodnoty aktualnej bunky na obrazovku
            s_load1,             -- , nacitanie vstupu do aktualnej bunky
            s_while_s1, s_while_s2, s_while_s3, s_while_s4,  -- [ zaciatok while
            s_while_e1, s_while_e2, s_while_e3, s_while_e4, s_while_e5, -- ] koniec while
            s_load_tmp, s_load_tmp2,    -- ! nacitanie premennej do aktualnej bunky
            s_store_tmp1, s_store_tmp2, -- $ ulozenie hodnoty aktualnej bunky do premennej
            s_null,              -- null ukoncenie programu
            s_others             -- ostatne
        );
    signal curr_state: fsm_state; -- aktualny stav
    signal next_state: fsm_state; -- nasledujuci stav

    type instr_type is
        (
            p_inc,     -- inkrementacia ukazovatela
            p_dec,     -- dekrementacia ukazovatela
            v_inc,     -- inkrementacia hodnoty bunky
            v_dec,     -- dekrementacia hodnoty bunky
            while_s,   -- zaciatok while
            while_e,   -- koniec while
            print,     -- vypis na obrazovku
            load,      -- nacitanie vstupu
            store_tmp, -- ulozenie bunky do premennej
            load_tmp,  -- nacitanie premennej do bunky
            halt,      -- stop = null
            nop        -- ine
        );
    signal instruction: instr_type;

begin
    -- zde dopiste vlastni VHDL kod
    
    -- pri tvorbe kodu reflektujte rady ze cviceni INP, zejmena mejte na pameti, ze 
    --   - nelze z vice procesu ovladat stejny signal,
    --   - je vhodne mit jeden proces pro popis jedne hardwarove komponenty, protoze pak
    --   - u synchronnich komponent obsahuje sensitivity list pouze CLK a RESET a 
    --   - u kombinacnich komponent obsahuje sensitivity list vsechny ctene signaly.

    -------- Program counter register --------
    pc_register_process: process(CLK, RESET)
    begin
        if (RESET = '1') then
            pc_reg <= (others => '0');
        elsif (CLK'event) and (CLK = '1') then
            if (pc_dec = '1') then
                pc_reg <= pc_reg - 1;
            elsif (pc_inc = '1') then
                pc_reg <= pc_reg + 1;
            elsif (pc_set = '1') then
                pc_reg <= (others => '0');
            end if;
        end if;
    end process;

    -------- Pointer register --------
    ptr_register_process: process(CLK, RESET)
    begin
        if (RESET = '1') then
            ptr_reg <= "1000000000000"; -- data ulozene na adrese 0x1000(h) == 1000000000000(b)
        elsif (CLK'event) and (CLK = '1') then
            if (ptr_dec = '1') then
                if (ptr_reg = "1000000000000") then -- cyklicky
                    ptr_reg <= "1111111111111";
                else
                    ptr_reg <= ptr_reg - 1;
                end if;
            elsif (ptr_inc = '1') then
                if (ptr_reg = "1111111111111") then -- cyklicky
                    ptr_reg <= "1000000000000";
                else                    
                    ptr_reg <= ptr_reg + 1;
                end if;
            elsif (ptr_set = '1') then
                ptr_reg <= "1000000000000";
            end if;
        end if;
    end process;

    -------- Counter register --------
    counter_register_process: process(CLK, RESET)
    begin
        if (RESET = '1') then
            cnt_reg <= (others => '0');
        elsif (CLK'event) and (CLK = '1') then
            if (cnt_dec = '1') then
                cnt_reg <= cnt_reg - 1;
            elsif (cnt_inc = '1') then
                cnt_reg <= cnt_reg + 1;
            elsif (cnt_set = '1') then
                cnt_reg <= (others => '0');
            elsif (cnt_set_one = '1') then
                cnt_reg <= "00000001";
            end if;
        end if;
    end process;

    -------- MX1 multiplexor --------
    with sel1 select
        DATA_ADDR <= pc_reg  when '0',
                     mx2_out when '1',
                     (others => '0') when others;
    
    -------- MX2 multiplexor --------
    with sel2 select
        mx2_out <= ptr_reg when '0',
                   "1000000000000" when '1',
                   (others => '0') when others;
    
    -------- MX3 multiplexor --------
    with sel3 select
        DATA_WDATA <= IN_DATA when "00",        -- zapis hodnoty zo vstupu
                      DATA_RDATA when "11",     -- zapis aktualnej hodnoty bunky
                      DATA_RDATA - 1 when "01", -- zapis dekrementovanej hodnoty bunky
                      DATA_RDATA + 1 when "10", -- zapis inkrementovanej hodnoty bunky
                      (others => '0') when others;

    -------- Instr. dekoder --------
    decode_process: process (DATA_RDATA)
    begin
        case (DATA_RDATA) is
            when X"3E"  => instruction <= p_inc; -- >
            when X"3C"  => instruction <= p_dec; -- <
            when X"2B"  => instruction <= v_inc; -- +
            when X"2D"  => instruction <= v_dec; -- -
            when X"5B"  => instruction <= while_s; -- [
            when X"5D"  => instruction <= while_e; -- ]
            when X"2E"  => instruction <= print; -- .
            when X"2C"  => instruction <= load; -- ,
            when X"24"  => instruction <= store_tmp; -- $
            when X"21"  => instruction <= load_tmp; -- !
            when X"00"  => instruction <= halt; -- null
            when others => instruction <= nop; -- ine
        end case;
    end process;

    ---------------------------------
    -------- Konecny automat --------
    ---------------------------------

    -- Zmena stavu current_state
    fsm_curr_state: process(CLK, RESET, EN)
    begin
        if (RESET = '1') then
            curr_state <= s_start;
        elsif (CLK'event) and (CLK = '1') then
            if (EN = '1') then
                curr_state <= next_state;
            end if;
        end if;
    end process;

    -- Zmena nasledujuceho stavu
    fsm_next_state: process(instruction, curr_state, cnt_reg, CLK, RESET, EN, IN_VLD, OUT_BUSY, DATA_RDATA)
    begin
        -- Init
        OUT_WE <= '0';
	    IN_REQ <= '0';
	    DATA_EN <= '0';
        DATA_RDWR <= '0';
        OUT_DATA <= (others => '0'); -- odstranenie latchu
        
	    pc_inc <= '0';
	    pc_dec <= '0';
        pc_set <= '0';
        
        ptr_inc <= '0';
		ptr_dec <= '0';
        ptr_set <= '0';
        
        cnt_inc <= '0';
		cnt_dec <= '0';
        cnt_set <= '0';
        cnt_set_one <= '0';
        
        sel1 <= '0';
        sel2 <= '0';
        sel3 <= "11";
        
        case curr_state is

            --------- state start
            when s_start =>
                next_state <= s_instr_fetch;
                pc_set <= '1';  -- PC <= 0
                ptr_set <= '1'; -- PTR <= 0x1000
                cnt_set <= '1'; -- CNT <= 0
           
            --------- state fetch
            when s_instr_fetch =>
                next_state <= s_instr_decode;
                DATA_EN <= '1';
		        DATA_RDWR <= '0';
                sel1 <= '0';
            
            --------- state decode
            when s_instr_decode =>
                case instruction is
                    when p_inc =>
                        next_state <= s_inc_ptr;
                    when p_dec =>
                        next_state <= s_dec_ptr;
                    when v_inc =>
                        next_state <= s_inc1;
                    when v_dec =>
                        next_state <= s_dec1;
                    when while_s =>
                        next_state <= s_while_s1;
                    when while_e =>
                        next_state <= s_while_e1;
                    when print =>
                        next_state <= s_print1;
                    when load =>
                        next_state <= s_load1;
                    when store_tmp =>
                        next_state <= s_store_tmp1;
                    when load_tmp =>
                        next_state <= s_load_tmp;
                    when halt =>
                        next_state <= s_null;
                    when others =>
                        next_state <= s_others;
                end case;

            --------- >
            when s_inc_ptr =>
                next_state <= s_instr_fetch;
                ptr_inc <= '1';
                pc_inc <= '1';
            
            --------- <
            when s_dec_ptr =>
                next_state <= s_instr_fetch;
                ptr_dec <= '1';
                pc_inc <= '1';

            --------- +
            when s_inc1 =>
                next_state <= s_inc2;
                DATA_EN <= '1';
                sel1 <= '1';
		        sel2 <= '0';
            when s_inc2 =>
                next_state <= s_instr_fetch;
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                sel1 <= '1';
                sel2 <= '0';
                sel3 <= "10";
                pc_inc <= '1';

	        --------- -
            when s_dec1 =>
                next_state <= s_dec2;
                DATA_EN <= '1';
                sel1 <= '1';
		        sel2 <= '0';
            when s_dec2 =>
                next_state <= s_instr_fetch;
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                sel1 <= '1';
                sel2 <= '0';
                sel3 <= "01";
                pc_inc <= '1';
            
            --------- .
            when s_print1 =>
                next_state <= s_print2;
                DATA_EN <= '1';
                sel1 <= '1';
		        sel2 <= '0';
            when s_print2 =>
                if (OUT_BUSY = '1') then
                    next_state <= s_print2;
                else
                    next_state <= s_instr_fetch;
                    OUT_WE <= '1';
                    OUT_DATA <= DATA_RDATA;
                    pc_inc <= '1';
                end if;

            --------- $
            when s_store_tmp1 =>
                next_state <= s_store_tmp2;
                sel1 <= '1';
                sel2 <= '0';
                DATA_EN <= '1';
                DATA_RDWR <= '0';
            when s_store_tmp2 =>
                next_state <= s_instr_fetch;
                sel1 <= '1';
                sel2 <= '1';
                sel3 <= "11";
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                pc_inc <= '1';

            --------- !
            when s_load_tmp =>
                next_state <= s_load_tmp2;
                sel1 <= '1';
                sel2 <= '1';
                DATA_EN <= '1';
                DATA_RDWR <= '0';
            when s_load_tmp2 =>
                next_state <= s_instr_fetch;
                sel1 <= '1';
                sel2 <= '0';
                sel3 <= "11";
                DATA_EN <= '1';
                DATA_RDWR <= '1';
                pc_inc <= '1';

            --------- ,
            when s_load1 =>
                IN_REQ <= '1';
                if (IN_VLD = '0') then
                    next_state <= s_load1;
                else
                    next_state <= s_instr_fetch;
                    sel1 <= '1';
                    sel2 <= '0';
                    sel3 <= "00";
                    DATA_EN <= '1';
                    DATA_RDWR <= '1';
                    pc_inc <= '1';
                end if;

            --------- [
            when s_while_s1 => 
                next_state <= s_while_s2;
                pc_inc <= '1';
                sel1 <= '1';
                DATA_EN <= '1';
            when s_while_s2 =>
                if (DATA_RDATA = X"00") then
                    cnt_inc <= '1';
                    next_state <= s_while_s3;
                else
                    next_state <= s_instr_fetch;
                end if;
            when s_while_s3 =>
                next_state <= s_while_s4;
                DATA_EN <= '1';
            when s_while_s4 => 
                DATA_EN <= '1';
                if (cnt_reg = X"00") then
                    next_state <= s_instr_fetch;
                else
                    if (DATA_RDATA = X"5B") then
                        cnt_inc <= '1';
                    elsif (DATA_RDATA = X"5D") then
                        cnt_dec <= '1';
                    end if;
                    pc_inc <= '1';
                    next_state <= s_while_s3;
                end if;
                    
            --------- ]
            when s_while_e1 =>
                next_state <= s_while_e2;
                sel1 <= '1';
                DATA_EN <= '1';
            when s_while_e2 =>
                if (DATA_RDATA = X"00") then
                    pc_inc <= '1';
                    next_state <= s_instr_fetch;
                else
                    pc_dec <= '1';
                    cnt_inc <= '1';
                    next_state <= s_while_e3;
                end if;
            when s_while_e3 =>
                DATA_EN <= '1';
                next_state <= s_while_e4;
            when s_while_e4 =>
                DATA_EN <= '1';
                if (cnt_reg = X"00") then
                    next_state <= s_instr_fetch;
                else
                    if DATA_RDATA = X"5D" then
                        cnt_inc <= '1';
                    elsif DATA_RDATA = X"5B" then
                        cnt_dec <= '1';
                    end if;
                    next_state <= s_while_e5;
                end if;
            when s_while_e5 =>
                DATA_EN <= '1';
                if cnt_reg = X"00" then
                    pc_inc <= '1';
                else
                    pc_dec <= '1';
                end if;
                next_state <= s_while_e3;

            --------- null - ukoncenie
            when s_null =>
                    next_state <= s_null;

            --------- ostatne
            when s_others =>
                next_state <= s_instr_fetch;
                pc_inc <= '1';
            
            --------- nedefinovane
            when others =>
                null;

        end case;
    end process;
end behavioral;

