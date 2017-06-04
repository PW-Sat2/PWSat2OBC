#ifndef LIBS_MISSION_INCLUDE_MISSION_IHASSTATE_H_
#define LIBS_MISSION_INCLUDE_MISSION_IHASSTATE_H_

#include <type_traits>

/**
 * @brief Interface marking object as statefull.
 * @ingroup StateDef
 */
template <typename State> struct IHasState
{
    /**
     * @brief Type of the state object.
     */
    typedef typename std::remove_reference<State>::type StateType;

    /**
     * @brief State accessor.
     * @return Reference to the state.
     */
    virtual const StateType& GetState() const noexcept = 0;

    /**
     * @brief State accessor.
     * @return Reference to the state.
     */
    virtual StateType& GetState() noexcept = 0;
};

#endif /* LIBS_MISSION_INCLUDE_MISSION_IHASSTATE_H_ */
