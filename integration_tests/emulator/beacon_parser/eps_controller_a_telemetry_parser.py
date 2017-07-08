from parser import Parser


class EPSControllerATelemetryParser(Parser):
    def __init__(self, tree_control):
        Parser.__init__(self, tree_control, 'Controller A')

    def get_bit_count(self):
        return 3*(4*12+6) + 6*10 + 2*8 + 4*10 + 8 + 2*12 + 8 + 16 + 32 + 4*10

    def parse(self, address, bits):
        self.append_word(address, bits, 'MPPT_X.SOL_VOLT', 12)
        self.append_word(address, bits, 'MPPT_X.SOL_CURR', 12)
        self.append_word(address, bits, 'MPPT_X.SOL_OUT_VOLT', 12)
        self.append_word(address, bits, 'MPPT_X.Temperature', 12)
        self.append_byte(address, bits, 'MPPT_X.State', 6)
        self.append_word(address, bits, 'MPPT_Y+.SOL_VOLT', 12)
        self.append_word(address, bits, 'MPPT_Y+.SOL_CURR', 12)
        self.append_word(address, bits, 'MPPT_Y+.SOL_OUT_VOLT', 12)
        self.append_word(address, bits, 'MPPT_Y+.Temperature', 12)
        self.append_byte(address, bits, 'MPPT_Y+.State', 6)
        self.append_word(address, bits, 'MPPT_Y-.SOL_VOLT', 12)
        self.append_word(address, bits, 'MPPT_Y-.SOL_CURR', 12)
        self.append_word(address, bits, 'MPPT_Y-.SOL_OUT_VOLT', 12)
        self.append_word(address, bits, 'MPPT_Y-.Temperature', 12)
        self.append_byte(address, bits, 'MPPT_Y-.State', 6)
        self.append_word(address, bits, 'DISTR.VOLT_3V3', 10)
        self.append_word(address, bits, 'DISTR.CURR_3V3', 10)
        self.append_word(address, bits, 'DISTR.VOLT_5V', 10)
        self.append_word(address, bits, 'DISTR.CURR_5V', 10)
        self.append_word(address, bits, 'DISTR.VOLT_VBAT', 10)
        self.append_word(address, bits, 'DISTR.CURR_VBAT', 10)
        self.append_byte(address, bits, 'DISTR.LCL_STATE', 8)
        self.append_byte(address, bits, 'DISTR.LCL_FLAGS', 8)
        self.append_word(address, bits, 'BATC.VOLT_A', 10)
        self.append_word(address, bits, 'BATC.CHRG_CURR', 10)
        self.append_word(address, bits, 'BATC.DCHRG_CURR', 10)
        self.append_word(address, bits, 'BATC.Temperature', 10)
        self.append_byte(address, bits, 'BATC.State', 8)
        self.append_word(address, bits, 'BP.Temperature A', 12)
        self.append_word(address, bits, 'BP.Temperature B', 12)
        self.append_byte(address, bits, 'Error Code', 8)
        self.append_word(address, bits, 'Power Cycle Count', 16)
        self.append_dword(address, bits, 'Uptime', 32)
        self.append_word(address, bits, 'Temperature', 10)
        self.append_word(address, bits, 'Other.Temperature', 10)
        self.append_word(address, bits, 'DCDC3V3.Temperature', 10)
        self.append_word(address, bits, 'DCDC5V.Temperature', 10)


