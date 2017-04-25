#include "MESI_protocol.h"
#include "../sim/mreq.h"
#include "../sim/sim.h"
#include "../sim/hash_table.h"

extern Simulator *Sim;

/*************************
 * Constructor/Destructor.
 *************************/
MESI_protocol::MESI_protocol(Hash_table *my_table, Hash_entry *my_entry)
    : Protocol (my_table, my_entry) {
    state = MESI_CACHE_I;
}

MESI_protocol::~MESI_protocol() {}

void MESI_protocol::dump(void) {
    const char *block_states[8] = {"X","I","I","I","S","S","E","M"};
    fprintf (stderr, "MESI_protocol - state: %s\n", block_states[state]);
}

void MESI_protocol::process_cache_request(Mreq *request) {
	switch (state) {
        case MESI_CACHE_I: do_cache_I(request); break;
        case MESI_CACHE_IS: do_cache_IS(request); break;
        case MESI_CACHE_IM: do_cache_IM(request); break;
        case MESI_CACHE_S: do_cache_S(request); break;
        case MESI_CACHE_SM: do_cache_SM(request); break;
        case MESI_CACHE_E: do_cache_E(request); break;
        case MESI_CACHE_M: do_cache_M(request); break;

        default:
            fatal_error ("Invalid Cache State for MESI Protocol\n");
        }
}

void MESI_protocol::process_snoop_request(Mreq *request) {
	switch (state) {
        case MESI_CACHE_I: do_snoop_I(request); break;
        case MESI_CACHE_IS: do_snoop_IS(request); break;
        case MESI_CACHE_IM: do_snoop_IM(request); break;
        case MESI_CACHE_S: do_snoop_S(request); break;
        case MESI_CACHE_SM: do_snoop_SM(request); break;
        case MESI_CACHE_E: do_snoop_E(request); break;
        case MESI_CACHE_M: do_snoop_M(request); break;

        default:
            fatal_error ("Invalid Cache State for MESI Protocol\n");
    }
}

inline void MESI_protocol::do_cache_I(Mreq *request) {
    switch (request->msg) {
        case LOAD:
            // Go to IS state to wait for data
            send_GETS(request->addr);
            state = MESI_CACHE_IS;
            Sim->cache_misses++;
            break;
        case STORE:
            // Go to IM state to wait for data on bus
            send_GETM(request->addr);
            state = MESI_CACHE_IM;
            Sim->cache_misses++;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: I state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_IS(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            request->print_msg(my_table->moduleID, "ERROR");
            fatal_error("Should only have one outstanding request per processor!");
        default:
            // Processor shouldn't be sending anything!
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: IS state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_IM(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            request->print_msg(my_table->moduleID, "ERROR");
            fatal_error("Should only have one outstanding request per processor!");
        default:
            // Processor shouldn't be sending anything!
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: IM state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_S(Mreq *request) {
    switch (request->msg) {
        case LOAD:
            // Stay in S state
            send_DATA_to_proc(request->addr);
            break;
        case STORE:
            // Go to SM state and wait for data
            send_GETM(request->addr);
            state = MESI_CACHE_SM;
            Sim->cache_misses++;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: S state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_SM(Mreq *request) {
    switch (request->msg) {
        case LOAD:
        case STORE:
            request->print_msg(my_table->moduleID, "ERROR");
            fatal_error("Should only have one outstanding request per processor!");
        default:
            // Processor shouldn't be sending anything!
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: SM state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_E(Mreq *request) {
    switch (request->msg) {
        case LOAD:
            // Stay in E state
            send_DATA_to_proc(request->addr);
            break;
        case STORE:
            // Silent upgrade to E (no bus broadcast!)
            send_DATA_to_proc(request->addr);
            state = MESI_CACHE_M;
            Sim->silent_upgrades++;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: S state shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_cache_M(Mreq *request) {
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

inline void MESI_protocol::do_snoop_I(Mreq *request) {
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

inline void MESI_protocol::do_snoop_IS(Mreq *request) {
    switch (request->msg) {
        case GETS:
        case GETM:
            // Ignore, wait for data
            break;
        case DATA:
            // Data received, pass to proc
            send_DATA_to_proc(request->addr);

            // If block isn't shared, go to E!
            if (!get_shared_line())
                state = MESI_CACHE_E;
            else
                state = MESI_CACHE_S;

            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: IS snoop shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_IM(Mreq *request) {
    switch (request->msg) {
        case GETS:
        case GETM:
            // Ignore, wait for data
            break;
        case DATA:
            // Data received, pass to proc and go to S
            send_DATA_to_proc(request->addr);
            state = MESI_CACHE_M;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: IM snoop shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_S(Mreq *request) {
    switch (request->msg) {
        case GETS:
            // Stay in S, upgrade line to shared
            set_shared_line();
            state = MESI_CACHE_S;
            break;
        case GETM:
            // Invalidate
            set_shared_line();
            state = MESI_CACHE_I;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: S snoop shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_SM(Mreq *request) {
    switch (request->msg) {
        case GETS:
        case GETM:
            // Ignore, wait for data
            // But set as shared b/c we have this block (in S)
            set_shared_line();
            break;
        case DATA:
            // Data received, pass to proc and go to M
            send_DATA_to_proc(request->addr);
            state = MESI_CACHE_M;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: SM snoop shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_E(Mreq *request) {
    switch (request->msg) {
        case GETS:
            // Downgrade to S, initiate $-$ transfer
            // Remember, no one knows we have this block!
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            state = MESI_CACHE_S;
            break;
        case GETM:
            // Invalidate, but transfer first
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            state = MESI_CACHE_I;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: E snoop shouldn't see this message\n");
    }
}

inline void MESI_protocol::do_snoop_M(Mreq *request) {
    switch (request->msg) {
        case GETS:
            // Move to S, INTERVENE
            // Send data to requesting cache directly ($-$ transfer)
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            state = MESI_CACHE_S;
            break;
        case GETM:
            // Invalidate and INTERVENE
            set_shared_line();
            send_DATA_on_bus(request->addr, request->src_mid);
            state = MESI_CACHE_I;
            break;
        default:
            request->print_msg (my_table->moduleID, "ERROR");
            fatal_error ("Client: M snoop shouldn't see this message\n");
    }
}
