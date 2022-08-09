#include "rid.h"

#include "port.h"
#include "ranging.h"
#include "stdio_d.h"

void rid_main()
{
	stdio_write("\r\nstarting rid\r\n");

	setup_DW1000RSTnIRQ(0);
	dw_main();
}
