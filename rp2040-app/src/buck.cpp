#include "buck.h"

buck::buck(float vset, float vmax, pid pid)
:v_set(v_set), pd(pid)
{
    pd->set();

}
