library ieee;
use ieee.std_logic_1164.all;
use ieee.numeric_std.all;
use work.seven_segment_pkg.all;

entity seven_segment_agent is
	generic (
		lamp_mode_common_anode : boolean := true;
		decimal_support : boolean := true;
		implementer : natural range 1 to 255 := 20;
		revision : natural range 0 to 255 := 0;
		signed_support : boolean := false;
		blank_zeros_support : boolean := false
	);
	port (
		clk : in std_logic;
		reset_n : in std_logic;
		address : in std_logic_vector(1 downto 0);
		read : in std_logic;
		readdata : out std_logic_vector(31 downto 0);
		write : in std_logic;
		writedata : in std_logic_vector(31 downto 0);
		lamps : out std_logic_vector(41 downto 0)
	);
end entity seven_segment_agent;

architecture rtl of seven_segment_agent is
	
	signal data: std_logic_vector(31 downto 0) := (others => '0');
	signal control: std_logic_vector(31 downto 0) := (others => '0');

	function concat_config(cfg: seven_segment_digit_array) 
	return std_logic_vector is
		variable result: std_logic_vector(41 downto 0);
	begin
		for i in 0 to 5 loop
			result(7*i+6 downto 7*i) := 
			cfg(i).g & cfg(i).f & cfg(i).e & cfg(i).d & 
			cfg(i).c & cfg(i).b & cfg(i).a;
		end loop;
		return result;
	end function;

begin
	funtime : process(clk)
		constant MAGIC_NUMBER: std_logic_vector(31 downto 0) := X"41445335";
		variable features: std_logic_vector(31 downto 0);
	begin
		if rising_edge(clk) then
			if reset_n = '0' then
				data <= (others => '0');
				control <= (others => '0');

			elsif read = '1' then
				case address is
					when "00" => readdata <= data;
					when "01" => readdata <= control;
					when "10" => 
						features := (others => '0');
						features(31 downto 24) := std_logic_vector(to_unsigned(implementer, 8));
						features(23 downto 16) := std_logic_vector(to_unsigned(revision, 8));
						if lamp_mode_common_anode then
							features(3) := '1';
						end if;
						if decimal_support then
							features(0) := '1';
						end if;
						if signed_support then
							features(1) := '1';
						end if;
						if blank_zeros_support then
							features(2) := '1';
						end if;
						readdata <= features;
					when "11" => readdata <= MAGIC_NUMBER;
					when others => readdata <= (others => '0');
				end case;

			elsif write = '1' then
				case address is
					when "00" => data <= writedata;
					when "01" => 
						control(0) <= writedata(0);
						if decimal_support then
							control(1) <= writedata(1);
							if blank_zeros_support then
								control(2) <= writedata(2);
							end if;
							if signed_support then
								control(3) <= writedata(3);
							end if;
						end if;
					when others => null;
				end case;
			end if;
		end if;
	end process;


	output_driver: process(data, control)

		variable hex_digits: seven_segment_digit_array;
		variable lamp_mode: lamp_configuration;
		variable data_to_display: std_logic_vector(31 downto 0);
	begin
	
		if lamp_mode_common_anode then
			lamp_mode := common_anode;
		else
			lamp_mode := common_cathode;
		end if;
	
		-- Determine what to display
		if decimal_support and control(1) = '1' then
			-- Handle decimal mode
			if signed_support and control(3) = '1' and data(15) = '1' then
				-- Negative number handling
				hex_digits(5) := negative_sign(lamp_mode);  -- Display minus sign
				data_to_display(15 downto 0) := std_logic_vector(unsigned(not data(15 downto 0)) + 1);  -- 2's complement
			else
				if blank_zeros_support and control(2) = '1' then
					hex_digits(5) := lamps_off(lamp_mode);
				else
					hex_digits(5) := get_hex_digit(0, lamp_mode);
				end if;
				data_to_display := data;
			end if;
			data_to_display(19 downto 0) := to_bcd(data_to_display(15 downto 0));
				
			-- Convert each decimal digit
			for i in 0 to 4 loop
				hex_digits(i) := get_hex_digit(to_integer(unsigned(data_to_display(4*i+3 downto 4*i))), lamp_mode);
			end loop;
	
		else
			-- Hexadecimal mode
			for i in 0 to 5 loop
				hex_digits(i) := get_hex_digit(to_integer(unsigned(data(4*i+3 downto 4*i))), lamp_mode);
			end loop;
		end if;
	
		if blank_zeros_support and control(2) = '1' and decimal_support and control(1) = '1' then
--			for i in 4 downto 1 loop  -- Don't blank last digit
--				if hex_digits(i) = get_hex_digit(0, lamp_mode) and 
--					hex_digits(i+1) = get_hex_digit(0, lamp_mode) then
--					hex_digits(i) := lamps_off(lamp_mode);
--				end if;
--			end loop;
			if hex_digits(4) = get_hex_digit(0, lamp_mode) then
				hex_digits(4) := lamps_off(lamp_mode);
			end if;
			
			for i in 3 downto 1 loop
				if 			hex_digits(i) = get_hex_digit(0, lamp_mode)
						and	hex_digits(i + 1) = lamps_off(lamp_mode) then
					hex_digits(i) := lamps_off(lamp_mode);
				end if;
			end loop;
		end if;
	
		if control(0) = '1' then
			lamps <= concat_config(hex_digits);
		else
			lamps <= (others => '1');  -- All segments off
		end if;
	end process output_driver;

end architecture rtl;



