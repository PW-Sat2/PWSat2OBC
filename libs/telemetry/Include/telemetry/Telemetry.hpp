#ifndef LIBS_STATE_TELEMETRY_HPP
#define LIBS_STATE_TELEMETRY_HPP

#pragma once

#include <cstdint>
#include <tuple>
#include "base/ITelemetryContainer.hpp"
#include "base/writer.h"
#include "fwd.hpp"
#include "traits.hpp"

/**
 * @defgroup telemetry_details Telemetry management details
 * @ingroup telemetry
 *
 * @brief This module contains telemetry management routines.
 */
namespace telemetry
{
    namespace details
    {
        /**
         * @ingroup telemetry_details
         * @brief This class implements the ITelemetryContainer interface for single telemetry element.
         *
         * In essence this class is an intermediary that on one side implements
         * external Telemetry interface while at the same time it forward all of the
         * telemetry container requests to the actual telemetry container without exposing
         * its internal organization & implementation details.
         * This class is abstract as by design the actual telemetry container type should be derived from
         * set of TelemetryContainer<Type> objects and provide them with storage & state tracking capabilities.
         *
         * @tparam Owner The type of the actual telemetry container class.
         * @tparam Type Type of the telemetry element supported by this class.
         */
        template <typename Owner, typename Type> struct TelemetryContainer : public ITelemetryContainer<Type>
        {
            /**
             * @brief This function returns modifiable reference to the telemetry container object.
             *
             * @return Reference to modifiable telemetry container object.
             */
            virtual Owner& GetOwner() = 0;

            /**
             * @brief This function returns reference to the telemetry container object.
             *
             * @return Reference to telemetry container object.
             */
            virtual const Owner& GetOwner() const = 0;

            /**
             * @brief This procedure returns reference to current telemetry element object.
             * @return Reference to current telemetry element object.
             */
            virtual const Type& Get() const final override;

            /**
             * @brief This procedure updates single telemetry element object.
             * @param[in] value New telemetry element value.
             *
             * This procedure uses Arg::IsDifferent method to determine whether the new value is different enough
             * to be considered major state change and needs to be saved.
             */
            virtual void Set(const Type& value) final override;

            /**
             * @brief This procedure updates single telemetry element object.
             * @param[in] value New telemetry element value.
             * This procedure bypasses state change tracking logic and modifies the value regardless
             * of the scale of the change. However as a result changes made by this procedure as considered
             * to be non significant and will never be included in the list of types eligible for saving.
             */
            virtual void SetVolatile(const Type& value) final override;
        };

        template <typename Owner, typename Type> const Type& TelemetryContainer<Owner, Type>::Get() const
        {
            return GetOwner().template Get<Type>();
        }

        template <typename Owner, typename Type> void TelemetryContainer<Owner, Type>::Set(const Type& value)
        {
            GetOwner().Set(value);
        }

        template <typename Owner, typename Type> void TelemetryContainer<Owner, Type>::SetVolatile(const Type& value)
        {
            GetOwner().SetVolatile(value);
        }

        /**
         * @brief Helper type that verifies whether the telemetry elements have unique identifiers.
         * @ingroup telemetry_details
         */
        template <typename... Args> struct AreIdsUnique;

        /**
         * @brief Helper type that verifies whether the telemetry elements have unique identifiers.
         * @ingroup telemetry_details
         *
         * @remark General case specialization for at least two types.
         */
        template <typename Base, typename... Args> struct AreIdsUnique<Base, Args...>
        {
            /**
             * @brief This value indicates whether the values of Type::Id for all types in the {Base, Args..} list are unique.
             */
            static constexpr bool Value = !IsValueInList<int>::IsInList<Base::Id, Args::Id...>() && AreIdsUnique<Args...>::Value;
        };

        /**
         * @brief Helper type that verifies whether the telemetry elements have unique identifiers.
         * @ingroup telemetry_details
         *
         * @remark Specialization for single type.
         */
        template <typename Base> struct AreIdsUnique<Base>
        {
            /**
             * @brief This value indicates that Base::Id is unique in current single element list.
             */
            static constexpr bool Value = true;
        };

        /**
         * @brief Helper type that calculates total serialized size for group of telemetry elements.
         * @ingroup telemetry_details
         */
        template <typename... Args> struct TotalSize;

        /**
         * @brief Helper type that calculates total serialized size for group of telemetry elements.
         * @ingroup telemetry_details
         *
         * @remark Specialization for at least two telemetry element types.
         */
        template <typename Base, typename... Args> struct TotalSize<Base, Args...>
        {
            /**
             * @brief This variable contains sum of serialized sizes of all types from Args collection plus Base type.
             */
            static constexpr int Value = Base::Size() + TotalSize<Args...>::Value;
        };

        /**
         * @brief Helper type that calculates total serialized size for group of telemetry elements.
         * @ingroup telemetry_details
         *
         * @remark Specialization for single telemetry element type.
         */
        template <typename Base> struct TotalSize<Base>
        {
            /**
             * @brief This variable contains total serialized size of Base type.
             */
            static constexpr int Value = Base::Size();
        };
    }

    /**
     * @brief Class that is responsible for keeping together complete satellite telemetry information.
     * @ingroup telemetry
     *
     * Besides keeping the elements together this type also keeps track of changes that are made to all
     * of the telemetry elements and when asked saves them to passed buffer as a list of state changes.
     *
     * @tparam Parts List of types that are considered to be part of the satellite telemetry.
     * There should not be any type duplicates on this list. Every type that is supposed to be part
     * of the telemetry should implement interface that is compatible to:
     * @code{.cpp}
     *
     * // @brief This type should be default constructible. Such state should represent valid telemetry object.
     * T();
     *
     * // @brief This static member is supposed to be type's unique identifier.
     * //
     * // This identifier will be used to indicate this specific type in the serialized list
     * // of telemetry changes.
     * static constexpr int Id;
     *
     * // @brief This function is responsible for reading complete state from the passed buffer reader object.
     * // @param[in] reader buffer reader that should be used to read the serialized state.
     * void Read(Reader& reader);
     *
     * // @brief The Write function is responsible for writing current object state to the passed writer object.
     * // @param[in] writer Buffer writer object that should be used to write the serialized state.
     * void Write(Writer& writer) const;
     *
     * // @brief This function is used to discriminate minor changes/fluctuations in telemetry state.
     * //
     * // The purpose of this function is to limit the number of state changes that are saved as the
     * // important telemetry changes in the telemetry event file.
     * //
     * // Since writing general purpose algorithm to determine whether telemetry change is significant
     * // or not is not possible the burden of determination whether the scale of the observed change
     * // in any particular telemetry part is placed on that telemetry part itself which should be
     * // be capable by itself to determine the necessary threshold and check if it has been crossed.
     * // @param[in] arg Previous observed telemetry state.
     * // @return Information whether the two objects differ enough to be considered different.
     * // @retval true The two objects are different.
     * // @retval false The two objects are considered to be equal.
     * bool IsDifferent(const Type& arg) const;
     *
     * // @brief This procedure is responsible to return size of this object in its serialized form.
     * //
     * // If the size is by definition variable, then this function should return highest possible size
     * // that is possible to be generated.
     * static constexpr std::uint32_t Size();
     * @endcode
     *
     */
    template <typename... Type> class Telemetry final : public details::TelemetryContainer<Telemetry<Type...>, Type>...
    {
      public:
        static_assert(AreTypesUnique<Type...>::value, "Telemetry types should be unique");

        static_assert(details::AreIdsUnique<Type...>::Value, "Telemetry type identifiers should be unique");

        /**
         * @brief This variable contains number of currently managed telemetry elements.
         */
        static constexpr int TypeCount = sizeof...(Type);

        /**
         * @brief This variable contains total serialized size of all telemetry elements.
         */
        static constexpr int PayloadSize = details::TotalSize<Type...>::Value;

        /**
         * @brief This variable contains serialized size of entire telemetry container.
         */
        static constexpr int TotalSerializedSize = PayloadSize + 2 * TypeCount * sizeof(std::uint8_t);

        virtual Telemetry<Type...>& GetOwner() final override;

        virtual const Telemetry<Type...>& GetOwner() const final override;

        /**
         * @brief This procedure updates single telemetry element object.
         * @param[in] arg New telemetry element value.
         * @tparam Arg Current Type of the telemetry element being set. This type should be available on the
         * list of telemetry elements.
         *
         * This procedure uses Arg::IsDifferent method to determine whether the new value is different enough
         * to be considered major state change and needs to be saved.
         */
        template <typename Arg> void Set(const Arg& arg);

        /**
         * @brief This procedure updates single telemetry element object.
         * @param[in] arg New telemetry element value.
         * @tparam Arg Current Type of the telemetry element being set. This type should be available on the
         * list of telemetry elements.
         * This procedure bypasses state change tracking logic and modifies the value regardless
         * of the scale of the change. However as a result changes made by this procedure as considered
         * to be non significant and will never be included in the list of types eligible for saving.
         */
        template <typename Arg> void SetVolatile(const Arg& arg);

        /**
         * @brief This procedure returns reference to queried telemetry element object.
         * @tparam Type of queried telemetry element.
         * @return Reference to queried telemetry element object.
         */
        template <typename Arg> const Arg& Get() const;

        /**
         * @brief This function returns information whether there is at least one modified telemetry element.
         * @return True if there is at least one modified telemetry element, false otherwise.
         */
        bool IsModified() const;

        /**
         * @brief This function is responsible for writing modified telemetry elements to the passed buffer writer.
         * @param[in] writer Buffer writer object that should be used to write the serialized state.
         */
        void WriteModified(Writer& writer) const;

        /**
         * @brief Informs telemetry container that all changes have been saved. And from now on the telemetry
         * elements should be considered unmodified.
         */
        void CommitCapture();

      private:
        /**
         * @brief Verifies whether the passed object is considered to be different from the one
         * currently stored.
         * @param[in] arg Object to be examined.
         * @return True if passed object is considered to be significantly different, false otherwise.
         */
        template <typename Arg> bool IsDifferent(const Arg& arg) const;

        /**
         * @brief Telemetry element wrapper that attaches precalculated information whether it
         * has been mofidied since last save.
         * @tparam Arg Telemetry element type.
         */
        template <typename Arg> using ElementContainer = std::pair<Arg, bool>;

        typedef std::tuple<ElementContainer<Type>...> Container;

        template <int Tag, typename T, typename... Args> bool IsModifiedInternal() const;

        template <int Tag> bool IsModifiedInternal() const;

        template <int Tag, typename T, typename... Args> void WriteModifiedInternal(Writer& writer) const;

        template <int Tag> void WriteModifiedInternal(Writer& writer) const;

        template <int Tag, typename T, typename... Args> void CommitCaptureInternal();

        template <int Tag> void CommitCaptureInternal();

        /**
         * @brief std::tuple that contains all the telemetry elements.
         */
        Container storage;
    };

    template <typename... Type> constexpr int Telemetry<Type...>::TypeCount;
    template <typename... Type> constexpr int Telemetry<Type...>::PayloadSize;
    template <typename... Type> constexpr int Telemetry<Type...>::TotalSerializedSize;

    template <typename... Type> Telemetry<Type...>& Telemetry<Type...>::GetOwner()
    {
        return *this;
    }

    template <typename... Type> const Telemetry<Type...>& Telemetry<Type...>::GetOwner() const
    {
        return *this;
    }

    template <typename... Type> template <typename Arg> void Telemetry<Type...>::Set(const Arg& arg)
    {
        if (IsDifferent(arg))
        {
            auto& entry = std::get<ElementContainer<Arg>>(this->storage);
            entry.first = arg;
            entry.second = true;
        }
    }

    template <typename... Type> template <typename Arg> void Telemetry<Type...>::SetVolatile(const Arg& arg)
    {
        if (IsDifferent(arg))
        {
            std::get<ElementContainer<Arg>>(this->storage).first = arg;
        }
    }

    template <typename... Type> template <typename Arg> const Arg& Telemetry<Type...>::Get() const
    {
        return std::get<ElementContainer<Arg>>(this->storage).first;
    }

    template <typename... Type> template <typename Arg> bool Telemetry<Type...>::IsDifferent(const Arg& arg) const
    {
        return std::get<ElementContainer<Arg>>(this->storage).first.IsDifferent(arg);
    }

    template <typename... Type> inline bool Telemetry<Type...>::IsModified() const
    {
        return IsModifiedInternal<0, Type...>();
    }

    template <typename... Type> template <int Tag, typename T, typename... Args> inline bool Telemetry<Type...>::IsModifiedInternal() const
    {
        return std::get<ElementContainer<T>>(this->storage).second || IsModifiedInternal<0, Args...>();
    }

    template <typename... Type> template <int Tag> inline bool Telemetry<Type...>::IsModifiedInternal() const
    {
        return false;
    }

    template <typename... Type> inline void Telemetry<Type...>::WriteModified(Writer& writer) const
    {
        return WriteModifiedInternal<0, Type...>(writer);
    }

    template <typename... Type> inline void Telemetry<Type...>::CommitCapture()
    {
        CommitCaptureInternal<0, Type...>();
    }

    template <typename... Type>
    template <int Tag, typename T, typename... Args>
    inline void Telemetry<Type...>::WriteModifiedInternal(Writer& writer) const
    {
        const auto& entry = std::get<ElementContainer<T>>(this->storage);
        if (entry.second)
        {
            writer.WriteByte(T::Id);
            entry.first.Write(writer);
            writer.WriteByte(T::Id);
        }

        WriteModifiedInternal<0, Args...>(writer);
    }

    template <typename... Type> template <int Tag> inline void Telemetry<Type...>::WriteModifiedInternal(Writer& /*writer*/) const
    {
    }

    template <typename... Type> template <int Tag, typename T, typename... Args> inline void Telemetry<Type...>::CommitCaptureInternal()
    {
        std::get<ElementContainer<T>>(this->storage).second = false;
        CommitCaptureInternal<0, Args...>();
    }

    template <typename... Type> template <int Tag> inline void Telemetry<Type...>::CommitCaptureInternal()
    {
    }
}

#endif
