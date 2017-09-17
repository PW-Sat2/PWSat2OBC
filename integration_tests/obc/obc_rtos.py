from .obc_mixin import OBCMixin, command, decode_return, decode_lines


class RTOSMixin(OBCMixin):
    def __init__(self):
        pass

    def task_list_parser(s):
        tasks = []
        lines = s.split('\n')
        for line in lines:
            tasks.append(line.split('\t'))

        return tasks

    @decode_return(task_list_parser)
    @command("tasklist")
    def tasklist(self):
        pass
