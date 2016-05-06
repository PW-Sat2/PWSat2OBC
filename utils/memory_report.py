import sys
import re

SEGMENTS = {
    'FLASH': {
        'size': 1 * 1024 * 1024,
        'sections': ['.text']
    },
    'RAM': {
        'size': 128 * 1024,
        'sections': ['.data', '.bss', '.heap']
    }
}

FLASH_SECTIONS = ['.text']
RAM_SECTIONS = []

class Section:
    def __init__(self, section, address, size):
        self.size = size
        self.start = address
        self.end = self.start + size
        self.name = section

    def __str__(self):
        return '%s 0x%X:0x%X' % (self.name, self.start, self.end)

    @classmethod
    def parse(cls, line):
        m = re.match(r"(?P<section>\S+)\s+(?P<size>\d+)\s+(?P<addr>\d+)", line)

        return Section(m.group('section'), int(m.group('addr')), int(m.group('size')))


def read_size_report(path):
    sections = {}

    with open(path, "r") as f:
        lines = f.readlines()[2:-3]

        for line in lines:
            section = Section.parse(line)
            sections[section.name] = section

    return sections


def report_segments_usage(sections):
    for segment_name in SEGMENTS:
        segment = SEGMENTS[segment_name]
        sections_in_segment = map(lambda s: sections[s], segment['sections'])

        used_size = sum(map(lambda s: s.size, sections_in_segment))
        percent_used = used_size / float(segment['size'])

        print '%s: Used: %.2f KB (%.2f %%)' % (segment_name, used_size/1024.0, percent_used * 100)


def main(path):
    sections = read_size_report(path)

    report_segments_usage(sections)


main(sys.argv[1])