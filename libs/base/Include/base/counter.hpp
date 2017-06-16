#ifndef LIBS_BASE_INCLUDE_BASE_COUNTER_HPP_
#define LIBS_BASE_INCLUDE_BASE_COUNTER_HPP_

namespace counter
{
    template <typename Type, Type Bottom, Type Top, Type Start = Bottom> class Counter
    {
      public:
        Counter();

        Type Value() const;
        bool IsTop() const;
        bool IsBottom() const;

        void GoUp();
        void GoDown();

        template <typename Action> void DoAndGoDown(Action action);
        template <typename Action> void DoAndGoUp(Action action);

      private:
        Type _value;
    };

    template <typename Type, Type Bottom, Type Top, Type Start> inline Counter<Type, Bottom, Top, Start>::Counter() : _value(Start)
    {
    }

    template <typename Type, Type Bottom, Type Top, Type Start> inline Type Counter<Type, Bottom, Top, Start>::Value() const
    {
        return this->_value;
    }

    template <typename Type, Type Bottom, Type Top, Type Start> inline bool Counter<Type, Bottom, Top, Start>::IsTop() const
    {
        return this->_value == Top;
    }

    template <typename Type, Type Bottom, Type Top, Type Start> inline bool Counter<Type, Bottom, Top, Start>::IsBottom() const
    {
        return this->_value == Bottom;
    }

    template <typename Type, Type Bottom, Type Top, Type Start> inline void Counter<Type, Bottom, Top, Start>::GoUp()
    {
        if (this->IsTop())
        {
            this->_value = Bottom;
        }
        else
        {
            this->_value++;
        }
    }

    template <typename Type, Type Bottom, Type Top, Type Start> inline void Counter<Type, Bottom, Top, Start>::GoDown()
    {
        if (this->IsBottom())
        {
            this->_value = Top;
        }
        else
        {
            this->_value--;
        }
    }

    template <typename Type, Type Bottom, Type Top, Type Start>
    template <typename Action>
    inline void Counter<Type, Bottom, Top, Start>::DoAndGoDown(Action action)
    {
        if (this->IsBottom())
        {
            action();
        }

        this->GoDown();
    }

    template <typename Type, Type Bottom, Type Top, Type Start>
    template <typename Action>
    inline void Counter<Type, Bottom, Top, Start>::DoAndGoUp(Action action)
    {
        if (this->IsTop())
        {
            action();
        }

        this->GoUp();
    }
}

#endif /* LIBS_BASE_INCLUDE_BASE_COUNTER_HPP_ */
