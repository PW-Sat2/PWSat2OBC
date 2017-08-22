#ifndef LIBS_MISSION_COMM_INCLUDE_MISSION_PERIODIC_MESSAGE_HPP_
#define LIBS_MISSION_COMM_INCLUDE_MISSION_PERIODIC_MESSAGE_HPP_

namespace mission
{
    /**
     * @brief Periodic message trigger
     * @ingroup mission_comm
     */
    struct ITriggerPeriodicMessage
    {
        /**
         * @brief Triggers periodic message
         */
        virtual void Trigger() = 0;
    };
}

#endif /* LIBS_MISSION_COMM_INCLUDE_MISSION_PERIODIC_MESSAGE_HPP_ */
