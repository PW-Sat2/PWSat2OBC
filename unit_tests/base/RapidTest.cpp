#include "gtest/gtest.h"
#include "gmock/gmock.h"
#include "rapidcheck.hpp"
#include "rapidcheck/gtest.h"

#include "system.h"
#include "base/reader.h"
#include "base/writer.h"

using testing::Eq;
using testing::ElementsAre;


template<typename WriterFunction, typename ReaderMemberPtr, typename V>
void WriterReaderRapidCheckTest(WriterFunction w, ReaderMemberPtr r, V value)
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

RC_GTEST_PROP(WriteReadRapicheck, Byte, (uint8_t value))
{
	WriterReaderRapidCheckTest(WriterWriteByte, &Reader::ReadByte, value);
}

RC_GTEST_PROP(WriteReadRapicheck, WordLE, (uint16_t value))
{
	WriterReaderRapidCheckTest(WriterWriteWordLE, &Reader::ReadWordLE, value);
}

RC_GTEST_PROP(WriteReadRapicheck, SignedWordLE, (int16_t value))
{
	WriterReaderRapidCheckTest(WriterWriteSignedWordLE, &Reader::ReadSignedWordLE, value);
}

RC_GTEST_PROP(WriteReadRapicheck, DoubleWordLE, (uint32_t value))
{
	WriterReaderRapidCheckTest(WriterWriteDoubleWordLE, &Reader::ReadDoubleWordLE, value);
}

RC_GTEST_PROP(WriteReadRapicheck, SignedDoubleWordLE, (int32_t value))
{
	WriterReaderRapidCheckTest(WriterWriteSignedDoubleWordLE, &Reader::ReadSignedDoubleWordLE, value);
}

RC_GTEST_PROP(WriteReadRapicheck, QuadWordLE, (uint64_t value))
{
	WriterReaderRapidCheckTest(WriterWriteQuadWordLE, &Reader::ReadQuadWordLE, value);
}

RC_GTEST_PROP(WriteReadRapicheck, Array, (std::vector<uint8_t> value))
{
	RC_PRE(value.size() > 0u);
	std::vector<uint8_t> array(value.size());

	Writer writer;
	WriterInitialize(&writer, array.data(), array.size());
	RC_ASSERT(WriterWriteArray(&writer, value.data(), value.size()));
	RC_ASSERT(WriterStatus(&writer));

	Reader reader{gsl::make_span(array.get(), value.size())};
	auto read = reader.ReadArray(value.size());
	for(size_t i = 0; i < value.size(); ++i)
	{
		RC_ASSERT(read[i] == value[i]);
	}
	RC_ASSERT(reader.Status());
	RC_ASSERT(reader.RemainingSize() == 0);
}
