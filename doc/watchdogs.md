# Watchdogs

In total there are 3 watchdogs available for OBC with different periods. They are used in three different places in order to achive mission safety.

| Watchdog | Period | Place | Guarantee |
|----------|--------|-------|-----------|
| Internal | ~5 seconds | Time service tick notification | Measuring (and passage) of time |
| External | ~1 seconds | FreeRTOS tick hook | Switching between tasks |
| EPS      | 5 minutes  | Mission loop | Mission loop running (so sail will eventually open) | 