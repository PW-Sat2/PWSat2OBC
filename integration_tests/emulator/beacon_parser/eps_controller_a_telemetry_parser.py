from parser import CategoryParser

class EPSControllerATelemetryParser(CategoryParser):
    def __init__(self, reader, store):
        CategoryParser.__init__(self, '14: Controller A', reader, store)

    def get_bit_count(self):
        return 3*(4*12+6) + 6*10 + 2*8 + 4*10 + 8 + 2*13 + 8 + 16 + 32 + 4*10 + 10

    def parse(self):
        self.append('MPPT_X.SOL_VOLT', 12)
        self.append('MPPT_X.SOL_CURR', 12)
        self.append('MPPT_X.SOL_OUT_VOLT', 12)
        self.append('MPPT_X.Temperature', 12)
        self.append('MPPT_X.State', 6)
        self.append('MPPT_Y+.SOL_VOLT', 12)
        self.append('MPPT_Y+.SOL_CURR', 12)
        self.append('MPPT_Y+.SOL_OUT_VOLT', 12)
        self.append('MPPT_Y+.Temperature', 12)
        self.append('MPPT_Y+.State', 6)
        self.append('MPPT_Y-.SOL_VOLT', 12)
        self.append('MPPT_Y-.SOL_CURR', 12)
        self.append('MPPT_Y-.SOL_OUT_VOLT', 12)
        self.append('MPPT_Y-.Temperature', 12)
        self.append('MPPT_Y-.State', 6)
        self.append('DISTR.VOLT_3V3', 10)
        self.append('DISTR.CURR_3V3', 10)
        self.append('DISTR.VOLT_5V', 10)
        self.append('DISTR.CURR_5V', 10)
        self.append('DISTR.VOLT_VBAT', 10)
        self.append('DISTR.CURR_VBAT', 10)
        self.append('DISTR.LCL_STATE', 8)
        self.append('DISTR.LCL_FLAGS', 8)
        self.append('BATC.VOLT_A', 10)
        self.append('BATC.CHRG_CURR', 10)
        self.append('BATC.DCHRG_CURR', 10)
        self.append('BATC.Temperature', 10)
        self.append('BATC.State', 8)
        self.append('BP.Temperature A', 13)
        self.append('BP.Temperature B', 13)
        self.append('Safety Counter', 8)
        self.append('Power Cycle Count', 16)
        self.append('Uptime', 32)
        self.append('Temperature', 10)
        self.append('SUPP_TEMP', 10)
        self.append('Other.Temperature', 10)
        self.append('DCDC3V3.Temperature', 10)
        self.append('DCDC5V.Temperature', 10)
