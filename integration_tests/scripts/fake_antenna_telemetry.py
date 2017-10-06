from utils import RepeatedTimer


def _update_times():
    state = [0]

    def to_raw(sec):
        return sec * (1000 / 50)

    def callback():
        i = state[0]
        system.primary_antenna.antenna_state[0].activation_time = to_raw(i + 0)
        system.primary_antenna.antenna_state[1].activation_time = to_raw(i + 2)
        system.primary_antenna.antenna_state[2].activation_time = to_raw(i + 4)
        system.primary_antenna.antenna_state[3].activation_time = to_raw(i + 8)

        system.backup_antenna.antenna_state[0].activation_time = to_raw(i + 8)
        system.backup_antenna.antenna_state[1].activation_time = to_raw(i + 4)
        system.backup_antenna.antenna_state[2].activation_time = to_raw(i + 2)
        system.backup_antenna.antenna_state[3].activation_time = to_raw(i + 0)

        state[0] += 1

    return callback


antenna_timer = RepeatedTimer(1, _update_times())
