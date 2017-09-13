RES = 1
TEMP = 0
OUT_OF_RANGE = -999

# Ni1000 relationship between resistances and temperatures [(temp0,resistance0), (temp1,resistance1), (tempN,resistanceN)]
ni1000_5000ppm_values = [(-80, 672.0), (-75, 692.0), (-70, 712.0), (-60, 751.8), (-50, 790.9), (-40, 830.8),
                         (-30, 871.7), (-20, 913.5), (-10, 956.2), (0, 1000.0),
                         (10, 1044.8), (20, 1090.7), (30, 1137.6), (40, 1185.7), (50, 1235.0), (60, 1285.4),
                         (70, 1337.1), (80, 1390.1), (90, 1444.4),
                         (100, 1500.0), (110, 1557.0), (120, 1615.4), (130, 1675.2), (140, 1736.5), (150, 1799.3),
                         (160, 1863.6), (170, 1929.5),
                         (180, 1997.0), (190, 2066.1), (200, 2137.0), (210, 2209.5), (220, 2283.7), (230, 2359.8),
                         (240, 2437.6), (250, 2517.3)]
ni1000_6180ppm_values = [(-70, 647.8), (-60, 695.2), (-50, 742.6), (-40, 791.3), (-30, 841.5), (-20, 893), (-10, 945.8),
                         (0, 1000.0),
                         (10, 1055.5), (20, 1112.4), (30, 1170.6), (40, 1230.1), (50, 1291.1), (60, 1353.4),
                         (70, 1417.2), (80, 1482.5), (90, 1549.3),
                         (100, 1617.8), (110, 1687.9), (120, 1759.7), (130, 1833.3), (140, 1908.9), (150, 1986.3),
                         (160, 2065.9), (170, 2147.6),
                         (180, 2231.5), (190, 2317.8), (200, 2406.6), (210, 2498), (220, 2592), (230, 2688.9),
                         (240, 2788.7), (250, 2891.6)]
pt1000_values = [(-70, 723.35), (-60, 763.28), (-50, 803.06), (-40, 842.71), (-30, 882.22), (-20, 921.6), (-10, 960.86),
                 (0, 1000),
                 (10, 1039), (20, 1077.9), (30, 1116.7), (40, 1155.4), (50, 1194), (60, 1232.4), (70, 1270.8),
                 (80, 1309), (90, 1347.1),
                 (100, 1385.1), (110, 1422.9), (120, 1460.7), (130, 1498.3), (140, 1535.8), (150, 1573.9),
                 (160, 1610.5), (170, 1447.7),
                 (180, 1684.8), (190, 1721.7), (200, 1758.6), (210, 1795.3), (220, 1831.9), (230, 1868.4),
                 (240, 1904.7), (250, 1941)]


# Public functions
def ni1000_5000ppm_res_to_temp(ni1000_resistance):
    """
    This function converts an Ni1000 5000ppm sensor resistance to temperature

    Parameters:
    ===========
    ni1000_resistance: Ni1000 5000ppm resistance in Ohms

    Return:
    ===========
    Ni1000 5000ppm resistance converted to temperature
    """
    return res_to_temp(ni1000_5000ppm_values, ni1000_resistance)


def pt1000_res_to_temp(pt1000_resistance):
    """
    This function converts an PT1000 sensor resistance to temperature

    Parameters:
    ===========
    pt1000_resistance: PT1000 resistance in Ohms

    Return:
    ===========
    PT1000 resistance converted to temperature
    """
    return res_to_temp(pt1000_values, pt1000_resistance)


# Public functions
def ni1000_6180ppm_res_to_temp(ni1000_resistance):
    """
    This function converts an Ni1000 6180ppm sensor resistance to temperature

    Parameters:
    ===========
    ni1000_resistance: Ni1000 6180ppm resistance in Ohms

    Return:
    ===========
    Ni1000 6180ppm resistance converted to temperature
    """
    return res_to_temp(ni1000_6180ppm_values, ni1000_resistance)


# Private functions
def res_to_temp(values_list, resistance):
    """
    This function converts a sensor resistance to temperature

    Parameters:
    ===========
    values_list: relationship between resistances and temperatures [(temp0,resistance0), (temp1,resistance1), (tempN,resistanceN)]
    resistance: a sensor resistance in Ohms

    Return:
    ===========
    Sensor resistance converted to temperature
    """
    first_resistance = values_list[0][RES]
    last_resistance = values_list[-1][RES]
    start_index = 0
    end_index = -1
    calculated_temp = OUT_OF_RANGE

    if (resistance >= first_resistance) and (resistance <= last_resistance):
        while values_list[start_index][RES] < resistance:
            start_index += 1
        while values_list[end_index][RES] > resistance:
            end_index -= 1

        delta_res = abs(values_list[start_index][RES] - values_list[end_index][RES])
        delta_temp = abs(values_list[start_index][TEMP] - values_list[end_index][TEMP])

        if delta_temp == 0:
            return values_list[start_index][TEMP]

        temp_coefficient = delta_res / delta_temp
        calculated_temp = ((resistance - values_list[end_index][RES]) / temp_coefficient) + values_list[end_index][TEMP]

    return calculated_temp