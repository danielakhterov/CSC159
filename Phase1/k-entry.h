#ifndef __K_ENTRY__
#define __K_ENTRY__

// Skip beloew if ASSEMBLER defined (from an assembly code)
#ifndef ASSEMBLER

__BEGIN_DECLS

#include "k-type.h"

void TimerEntry(void);
void Loader(trapframe_t *);

__END_DECLS

#endif

#endif
