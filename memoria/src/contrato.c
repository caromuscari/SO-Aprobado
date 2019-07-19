#include "contrato.h"

void destroyStMessageResponse(st_messageResponse *stMR) {
	free(stMR->buffer);
	free(stMR);
}
