## Led
set_property -dict {PACKAGE_PIN R14 IOSTANDARD LVCMOS33} [get_ports {GPO}]

## Speaker
set_property -dict {PACKAGE_PIN R16 IOSTANDARD LVCMOS33} [get_ports { CLK_OUT }]

# Clock signal 125 MHz
set_property -dict { PACKAGE_PIN H16   IOSTANDARD LVCMOS33 } [get_ports { CLK_IN }];
create_clock -add -name sys_clk_pin -period 8.00 -waveform {0 5} [get_ports { CLK_IN }];
