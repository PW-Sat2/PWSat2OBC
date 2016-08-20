import sys
import re
import path

SECTION_HEADER = r"^(?P<name>\S+)\s+(?P<base>0x[0-9a-f]+)\s+(?P<size>0x[0-9a-f]+)$"
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


def find_section_slice(lines, section_name):
    sections = skip_until(lines, lambda x: not x.startswith(section_name))

    results = {}
    current_section = []

    for line in sections:
        if line.strip() == '':
            continue

        header_match = re.match(SECTION_HEADER, line)

        if header_match is not None:
            name = header_match.group('name')
            current_section = []
            results[name] = {
                'header': name,
                'base': header_match.group('base'),
                'size': header_match.group('size'),
                'contents': current_section
            }
        else:
            current_section.append(line)

    return results


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
            per_archive[archive] = 0

        per_archive[archive] += r['size']

    return per_archive


with open(sys.argv[1], 'r') as f:
    memory_map = f.readlines()


slice = find_section_slice(memory_map, '.text')

text_section = slice['.text']

results = symbols_sizes(text_section['contents'])

size_per_archive = group_by_archive(results)

sum = 0

for a in sorted(size_per_archive):
    sum += size_per_archive[a]
    print "%s -> %d" % (a, size_per_archive[a])

print (sum/1024.0)
