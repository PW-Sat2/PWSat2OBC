import sys
import re
import os
import pybars
import path

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

SECTION_HEADER = r"^(?P<name>\S+)\s+(?P<base>0x[0-9a-f]+)\s+(?P<size>0x[0-9a-f]+)"
SECTION_ARCHIVE = r"^\s+(?P<section_name>\S+)?\s*(?P<base>0x[0-9a-f]+)\s+(?P<size>0x[0-9a-f]+)\s+(?P<archive>.*?)(\((?P<object>\S+?)\))?$"


def skip_until(items, predicate):
    yield_rest = False

    for item in items:
        if yield_rest:
            yield item
            continue

        if not predicate(item):
            yield_rest = True
            yield item


def take_until(items, predicate):
    for item in items:
        if predicate(item):
            yield item
        else:
            return

def symbols_sizes(section):
    for line in section:
        archive = re.match(SECTION_ARCHIVE, line)
        if archive is None:
            continue

        yield {
            'section_name': archive.group('section_name'),
            'base': archive.group('base'),
            'size': int(archive.group('size'), 16),
            'archive': archive.group('archive'),
            'object': archive.group('object')
        }


def group_by_archive(symbols):
    per_archive = {}

    for r in symbols:
        archive = str(path.path(r['archive']).basename())
        if archive == '':
            archive = r['section_name']

        if not per_archive.has_key(archive):
            per_archive[archive] = {'archive': archive, 'symbols': [], 'size': 0}

        per_archive[archive]['symbols'].append(r)
        per_archive[archive]['size'] += r['size']

    for a in per_archive.values():
        a['symbols'].sort(key=lambda s: -s['size'])

    return sorted(per_archive.values(), key=lambda a: -a['size'])


def find_section_slice(lines, section_name):
    sections = skip_until(lines, lambda x: not x.startswith(section_name))

    result = None
    i = 0
    for line in sections:
        i += 1
        if line.strip() == '':
            continue

        header_match = re.match(SECTION_HEADER, line)

        if header_match is not None:
            name = header_match.group('name')

            if name != section_name:
                break

            result = {
                'header': name,
                'base': header_match.group('base'),
                'size': int(header_match.group('size'), 16),
                'contents': []
            }
        else:
            result['contents'].append(line)

    result['symbols'] = symbols_sizes(result['contents'])
    result['archives'] = group_by_archive(result['symbols'])

    return result


def format_kilobytes(_, value):
    return '%.2f KB' % (value / 1024.0)


def format_percent(_, value, total):
    return '%.2f%%' % (value / float(total) * 100)


def report_segments_usage(memory_map, report_dir):
    model = {'segments': []}

    for segment_name in SEGMENTS:
        segment = SEGMENTS[segment_name]
        sections_in_segment = map(lambda s: find_section_slice(memory_map, s) or {'size': 0}, segment['sections'])

        total = float(segment['size'])

        used_size = sum(map(lambda s: s['size'], sections_in_segment))
        percent_used = used_size / total

        free_size = (total - used_size)

        model['segments'].append({
            'name': segment_name,
            'total': total,
            'used': used_size,
            'free': free_size,
            'sections': sections_in_segment
        })

        print '%s: Used: %.2f KB (%.2f %%)' % (segment_name, used_size/1024.0, percent_used * 100)

    if not os.path.exists(report_dir):
        os.makedirs(report_dir)

    with open(os.path.dirname(os.path.realpath(__file__)) + "/memory_report.mustache", 'r') as f:
        compiler = pybars.Compiler()
        template = compiler.compile(unicode(f.read()))

    with open(report_dir + '/index.html', 'w') as f:
        output = template(model, helpers={
            'kb': format_kilobytes,
            'percent': format_percent
        })
        f.write(unicode(output))


def main(memory_map_file, report_dir):
    with open(memory_map_file, 'r') as f:
        memory_map = f.readlines()

    report_segments_usage(memory_map, report_dir)


main(sys.argv[1], sys.argv[2])
