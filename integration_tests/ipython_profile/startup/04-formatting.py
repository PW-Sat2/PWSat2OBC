from datetime import timedelta

get_ipython().display_formatter.formatters['text/plain'].for_type(timedelta, lambda arg, p, cycle: p.text(str(arg)))