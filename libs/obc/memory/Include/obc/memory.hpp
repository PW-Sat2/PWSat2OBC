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
    };
}

#endif /* LIBS_OBC_MEMORY_INCLUDE_OBC_MEMORY_HPP_ */
