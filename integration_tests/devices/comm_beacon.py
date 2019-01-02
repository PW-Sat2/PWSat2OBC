from struct import pack

from bitarray import bitarray

from emulator.beacon_parser.full_beacon_parser import FullBeaconParser
from emulator.beacon_parser.parser import BitReader, BeaconStorage

import experiment_type

class BeaconFrame(object):
    def __init__(self, payload):
        self._payload = payload
        try:
            all_bits = bitarray(endian='little')
            all_bits.frombytes(''.join(map(lambda x: pack('B', x), payload)))

            reader = BitReader(all_bits)
            store = BeaconStorage()

            parsers = FullBeaconParser().GetParsers(reader, store)
            parsers.reverse()

            while len(parsers) > 0:
                parser = parsers.pop()
                parser.parse()

            self._parsed = store.storage
        except:
            self._parsed = None

    def payload(self):
        return self._payload

    def _get_proper_crc(self):
        try:
            from build_config import config
            return config['VALID_CRC']
        except Exception as e:
            return ""

    def __repr__(self):
        if self._parsed is None:
            return '{} (parse failed)'.format(self.__class__.__name__)

        proper_crc = self._get_proper_crc()

        v = lambda group, key: str(self._parsed[group][key])

        check_range = lambda group, key, low, high: low <= self._parsed[group][key].converted and self._parsed[group][key].converted <= high

        crc_ok = v('02: Program State', '0056: Program CRC') == proper_crc

        bp_volt_ok = '      '
        if not check_range('14: Controller A', '1019: BATC.VOLT_A', 7.0, 7.8) \
                or not check_range('15: Controller B', '1204: BATC.VOLT_B', 7.0, 7.8):
            bp_volt_ok = "!!!!! "

        bp_temp_ok = '      '
        if not check_range('14: Controller A', '1062: BP.Temperature A', 0, 45) \
                or not check_range('14: Controller A', '1075: BP.Temperature B', 0, 45)\
                or not check_range('15: Controller B', '1194: BP.Temperature', 0, 45):
            bp_temp_ok = "!!!!! "

        pa_temp_ok = '      '
        if not check_range('11: Comm', '0756: [Now] Power Amplifier Temperature', 0, 45) \
                or not check_range('11: Comm', '0605: [Last transmission] Power Amplifier Temperature', 0, 56):
            pa_temp_ok = "!!!!! "

        rx_current_ok = '      '
        if check_range('11: Comm', '0720: [Now] Receiver Current', 68, 75):
            rx_current_ok = "!!!!! "

        transmitter_bitrate = v('11: Comm', '0591: Transmitter Bitrate')

        experiment_status = ''
        experiment_startup_status = ''
        experiment_status_ok = '      '
        if v('09: Experiments', '0490: Current experiment code').strip() != 'None':
            experiment_status = ' {}'.format(v('09: Experiments', '0502: Last Experiment Iteration Status'))
            if experiment_status.strip() == 'Failure':
                experiment_status_ok = "!!!!! "

            if v('09: Experiments', '0494: Experiment Startup Result').strip() == 'Failure':
                experiment_startup_status = ' START Failure'
                if experiment_status_ok == '      ':
                    experiment_status_ok = '!!!!! '

        mcu_temperature_ok = '      '
        mcu_temperature = self._parsed['13: MCU']['0781: Temperature']
        gyro_temperature = self._parsed['10: Gyroscope']['0558: Temperature']

        if mcu_temperature.converted > gyro_temperature.converted + 10:
            mcu_temperature_ok = "!!!!! "

        current_3v3_ok = '      '
        current_3v3 = self._parsed['14: Controller A']['0956: DISTR.CURR_3V3']
        if current_3v3.converted > 0.1:
            current_3v3_ok = "!!!!! "

        lines = [
            '{} @ {} bps'.format(self.__class__.__name__, transmitter_bitrate),
            '{}BP VOLT {}, {}'.format(
                bp_volt_ok,
                v('14: Controller A', '1019: BATC.VOLT_A'),
                v('15: Controller B', '1204: BATC.VOLT_B')
            ),
            '{}BP TEMP {}, {}, {}'.format(
                bp_temp_ok,
                v('14: Controller A', '1062: BP.Temperature A'),
                v('14: Controller A', '1075: BP.Temperature B'),
                v('15: Controller B', '1194: BP.Temperature'),
            ),
            '{}COMM PA TEMP NOW {} LAST {}'.format(
                pa_temp_ok,
                v('11: Comm', '0756: [Now] Power Amplifier Temperature'),
                v('11: Comm', '0605: [Last transmission] Power Amplifier Temperature')
            ),
            '{}COMM RX Current {}'.format(
                rx_current_ok,
                v('11: Comm', '0720: [Now] Receiver Current'),
            )]
        if not crc_ok:
            lines.append(
            '!!!!! OBC CRC {}'.format(
                v('02: Program State', '0056: Program CRC')
            ))
        lines.extend([
            '      GYRO UNCAL {}, {}, {}'.format(
                v('10: Gyroscope', '0510: X measurement'),
                v('10: Gyroscope', '0526: Y measurement'),
                v('10: Gyroscope', '0542: Z measurement')
            ),
            '{}EXPERIMENT {}{}{}'.format(
                experiment_status_ok,
                v('09: Experiments', '0490: Current experiment code'),
                experiment_status,
                experiment_startup_status
            ),
            '{}MCU TEMP {}'.format(
                mcu_temperature_ok,
                mcu_temperature,
            ),
            '{}3V3 DISTR CURRENT {:.0f} mA\n'.format(
                current_3v3_ok,
                1000.*current_3v3.converted,
            )
        ])

        return '\n'.join(lines)
