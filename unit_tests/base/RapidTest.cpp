#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"

#include "base/reader.h"
#include "base/writer.h"

template<typename WriterFunction, typename ReaderMemberPtr, typename V>
void WriteAndReadRapidCheckTest(WriterFunction w, ReaderMemberPtr r, V value)
{
	Writer writer;
	std::array<uint8_t, sizeof(V)> array;

	WriterInitialize(&writer, array.begin(), array.size());
	RC_ASSERT(w(&writer, value));
	RC_ASSERT(WriterStatus(&writer));
	RC_ASSERT(WriterRemainingSize(&writer) == 0);

	Reader reader{gsl::make_span(array)};
	V read = (reader.*r)();
	RC_ASSERT(read == value);
	RC_ASSERT(reader.Status());
	RC_ASSERT(reader.RemainingSize() == 0);
}

RC_GTEST_PROP(WriteAndRead, Byte, (uint8_t value))
{
	WriteAndReadRapidCheckTest(WriterWriteByte, &Reader::ReadByte, value);
}

RC_GTEST_PROP(WriteAndRead, WordLE, (uint16_t value))
{
	WriteAndReadRapidCheckTest(WriterWriteWordLE, &Reader::ReadWordLE, value);
}

RC_GTEST_PROP(WriteAndRead, SignedWordLE, (int16_t value))
{
	WriteAndReadRapidCheckTest(WriterWriteSignedWordLE, &Reader::ReadSignedWordLE, value);
}

RC_GTEST_PROP(WriteAndRead, DoubleWordLE, (uint32_t value))
{
	WriteAndReadRapidCheckTest(WriterWriteDoubleWordLE, &Reader::ReadDoubleWordLE, value);
}

RC_GTEST_PROP(WriteAndRead, SignedDoubleWordLE, (int32_t value))
{
	WriteAndReadRapidCheckTest(WriterWriteSignedDoubleWordLE, &Reader::ReadSignedDoubleWordLE, value);
}

RC_GTEST_PROP(WriteAndRead, QuadWordLE, (uint64_t value))
{
	WriteAndReadRapidCheckTest(WriterWriteQuadWordLE, &Reader::ReadQuadWordLE, value);
}

RC_GTEST_PROP(WriteAndRead, Array, (std::vector<uint8_t> value))
{
	RC_PRE(value.size() > 0u);
	std::vector<uint8_t> array(value.size());

	Writer writer;
	WriterInitialize(&writer, array.data(), array.size());
	RC_ASSERT(WriterWriteArray(&writer, value.data(), value.size()));
	RC_ASSERT(WriterStatus(&writer));

	Reader reader{array};
	auto read = reader.ReadArray(value.size());
	for(size_t i = 0; i < value.size(); ++i)
	{
		RC_ASSERT(read[i] == value[i]);
	}
	RC_ASSERT(reader.Status());
	RC_ASSERT(reader.RemainingSize() == 0);
}
