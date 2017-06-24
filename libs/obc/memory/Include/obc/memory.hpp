#ifndef LIBS_OBC_MEMORY_INCLUDE_OBC_MEMORY_HPP_
#define LIBS_OBC_MEMORY_INCLUDE_OBC_MEMORY_HPP_

namespace obc
{
    enum class MemoryChip
    {
        SRAM1,
        SRAM2
    };

    class OBCMemory
    {
      public:
        void HandleLatchup(MemoryChip memory);
        void Recover();

      private:
        bool _sram1Latched = false;
        bool _sram2Latched = false;
    };
}

#endif /* LIBS_OBC_MEMORY_INCLUDE_OBC_MEMORY_HPP_ */
