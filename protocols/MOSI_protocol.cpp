#include "MOSI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MOSI_protocol::MOSI_protocol(Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry) {
    state = MOSI_CACHE_I;
}

MOSI_protocol::~MOSI_protocol() {}

void MOSI_protocol::dump(void) {
    const char *block_states[] = {"X","I","I","I","S","S","O","O","M"};
    fprintf (stderr, "MOSI_protocol - state: %s\n", block_states[state]);
}

void MOSI_protocol::process_cache_request (Mreq *request) {
	switch (state) {
        case MOSI_CACHE_I: do_cache_I(request); break;
        case MOSI_CACHE_IS: do_cache_IS(request); break;
        case MOSI_CACHE_IM: do_cache_IM(request); break;
        case MOSI_CACHE_S: do_cache_S(request); break;
        case MOSI_CACHE_SM: do_cache_SM(request); break;
        case MOSI_CACHE_O: do_cache_O(request); break;
        case MOSI_CACHE_OM: do_cache_OM(request); break;
        case MOSI_CACHE_M: do_cache_M(request); break;

        default:
            fatal_error ("Invalid Cache State for MOSI Protocol\n");
    }
}

void MOSI_protocol::process_snoop_request (Mreq *request) {
	switch (state) {
        case MOSI_CACHE_I: do_snoop_I(request); break;
        case MOSI_CACHE_IS: do_snoop_IS(request); break;
        case MOSI_CACHE_IM: do_snoop_IM(request); break;
        case MOSI_CACHE_S: do_snoop_S(request); break;
        case MOSI_CACHE_SM: do_snoop_SM(request); break;
        case MOSI_CACHE_O: do_snoop_O(request); break;
        case MOSI_CACHE_OM: do_snoop_OM(request); break;
        case MOSI_CACHE_M: do_snoop_M(request); break;

        default:
            fatal_error("Invalid Cache State for MOSI Protocol\n");
    }
}

inline void MOSI_protocol::do_cache_I(Mreq *request) {
    switch (request->msg) {
        case LOAD:
            // Go to IS state to wait for data
            send_GETS(request->addr);
            state = MOSI_CACHE_IS;
            Sim->cache_misses++;
            break;
        case STORE:
            // Go to IM state to wait for data on bus
            send_GETM(request->addr);
            state = MOSI_CACHE_IM;
            Sim->cache_misses++;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error("Client: I state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_cache_IS(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            request->print_msg(my_table->moduleID, "ERROR");
            fatal_error("Should only have one outstanding request per processor!");
        default:
            // Processor shouldn't be sending anything!
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error("Client: IS state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_cache_IM(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            request->print_msg(my_table->moduleID, "ERROR");
            fatal_error("Should only have one outstanding request per processor!");
        default:
            // Processor shouldn't be sending anything!
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error("Client: IM state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_cache_S(Mreq *request) {
    switch (request->msg) {
        case LOAD:
            // Stay in S state
            send_DATA_to_proc(request->addr);
            break;
        case STORE:
            // Go to SM state and wait for data
            send_GETM(request->addr);
            state = MOSI_CACHE_SM;
            Sim->cache_misses++;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: S state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_cache_SM(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            request->print_msg(my_table->moduleID, "ERROR");
            fatal_error("Should only have one outstanding request per processor!");
        default:
            // Processor shouldn't be sending anything!
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error("Client: SM state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_cache_O(Mreq *request) {
    switch (request->msg) {
        case LOAD:
            // Stay in O state
            send_DATA_to_proc(request->addr);
            break;
        case STORE:
            // Go to OM state and wait for data
            send_GETM(request->addr);
            state = MOSI_CACHE_OM;
            Sim->cache_misses++;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: S state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_cache_OM(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            request->print_msg(my_table->moduleID, "ERROR");
            fatal_error("Should only have one outstanding request per processor!");
        default:
            // Processor shouldn't be sending anything!
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error("Client: OM state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_cache_M(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            // Stay in M state
            // Pass data to proc
            send_DATA_to_proc(request->addr);
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: M state shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_I(Mreq *request) {
    switch (request->msg) {
        case GETS:
        case GETM:
        case DATA:
            // Already in I!
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: I snoop shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_IS(Mreq *request) {
    switch (request->msg) {
        case GETS:
        case GETM:
            // Ignore, wait for data
            break;
        case DATA:
            // Data received, pass to proc and go to S
            send_DATA_to_proc(request->addr);
            state = MOSI_CACHE_S;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: IS snoop shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_IM(Mreq *request) {
    switch (request->msg) {
        case GETS:
        case GETM:
            // Ignore, wait for data
            break;
        case DATA:
            // Data received, pass to proc and go to S
            send_DATA_to_proc(request->addr);
            state = MOSI_CACHE_M;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: IM/SM snoop shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_S(Mreq *request) {
    switch (request->msg) {
        case GETS:
            // Stay in S, upgrade line to shared
            set_shared_line();
            state = MOSI_CACHE_S;
            break;
        case GETM:
            // Invalidate
            set_shared_line();
            state = MOSI_CACHE_I;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: S snoop shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_SM(Mreq *request) {
    // Same as IM step!
    do_snoop_IM(request);
}

inline void MOSI_protocol::do_snoop_O(Mreq *request) {
    switch (request->msg) {
        case GETS:
            // Stay in O, upgrade line to shared
            // Also, perform xfer
            set_shared_line();
            state = MOSI_CACHE_O;
            send_DATA_on_bus(request->addr, request->src_mid);
            break;
        case GETM:
            // Invalidate
            send_DATA_on_bus(request->addr, request->src_mid);
            set_shared_line();
            state = MOSI_CACHE_I;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: O snoop shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_OM(Mreq *request) {
    switch (request->msg) {
        case GETS:
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            break;
        case GETM:
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            state = MOSI_CACHE_IM;
            break;
        case DATA:
            send_DATA_to_proc(request->addr);
            state = MOSI_CACHE_M;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: OM snoop shouldn't see this message\n");
    }
}

inline void MOSI_protocol::do_snoop_M(Mreq *request) {
    switch (request->msg) {
        case GETS:
            // Move to O, INTERVENE
            // Send data to requesting cache directly ($-$ transfer)
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            state = MOSI_CACHE_O;
            break;
        case GETM:
            // Invalidate and INTERVENE
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            state = MOSI_CACHE_I;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: M snoop shouldn't see this message\n");
    }
}
