#ifndef LIBS_STATE_INCLUDE_STATE_COMM_MESSAGESTATE_HPP_
#define LIBS_STATE_INCLUDE_STATE_COMM_MESSAGESTATE_HPP_

#include <chrono>
#include <cstdint>
#include "base/fwd.hpp"
#include "gsl/span"

namespace state
{
    /**
     * @brief This type represents persistent message settings
     * @persistent_state
     */
    class MessageState final
    {
      public:
        /**
         * @brief Ctor
         */
        MessageState();

        /**
         * @brief Ctor
         * @param interval Interval between sending messages (default 5 minutes)
         * @param repeatCount Number of message to send at once (default 3)
         * @param message Message payload (default: Borg)
         */
        MessageState(std::chrono::minutes interval, std::uint8_t repeatCount, gsl::span<const std::uint8_t> message);

        /**
         * @brief Returns interval between sending messages
         * @return Interval between sending messages
         */
        std::chrono::minutes Interval() const;

        /**
         * @brief Returns number of message to send at once
         * @return Number of message to send at once
         */
        std::uint8_t RepeatCount() const;

        /**
         * @brief Returns message payload
         * @return Message payload
         */
        gsl::span<const std::uint8_t> Message() const;

        /**
         * @brief Serializes object into given writer
         * @param writer Writer
         */
        void Write(Writer& writer) const;

        /**
         * @brief Deserializes object from reader
         * @param reader Reader
         */
        void Read(Reader& reader);

        /**
         * @brief Returns size of serialized object in bytes
         * @return Size in bytes
         */
        static constexpr std::uint32_t Size();

      private:
        /** @brief Interval between sending messages */
        std::chrono::minutes _interval;
        /** @brief Number of message to send at once */
        std::uint8_t _repeatCount;
        /** @brief Message payload */
        std::array<std::uint8_t, 200> _message;
    };

    constexpr std::uint32_t MessageState::Size()
    {
        return sizeof(std::uint8_t) + sizeof(std::uint8_t) + 200;
    }
}

#endif /* LIBS_STATE_INCLUDE_STATE_COMM_MESSAGESTATE_HPP_ */
