#include "url.h"
#include <fra/core.h>

#include "ht.h"
#include "end.h"
#include "dbg.h"
#include "lock.h"
#include "config.h"

#include <stdlib.h>
#ifndef NO_PTHREADS
#include <pthread.h>
#endif




// private functions and stuff :)

#ifndef NO_PTHREADS
static pthread_mutex_t urls_lock;
#endif
static fra_p_ht_t * urls = NULL;

struct url {
	struct url * next;
	bstring verb;
	fra_end_t * e;
};

static int set_url( void * value_v, void * arg_v ) {

        struct url * value;
        struct url * arg;


        value = (struct url *)value_v;

        arg = (struct url *)arg_v;

	value->next = NULL;
	value->verb = arg->verb;
	value->e = arg->e;

        return 0;

}

static void destroy_url( void * value_v ) {

	struct url * value;
	struct url * prev_value;


	if( value_v ) {

		value = (struct url *)value_v;

		bdestroy( value->verb );
		value = value->next;

		while( value ) {

			bdestroy( value->verb );

			prev_value = value;

			value = value->next;

			free( prev_value );

		}

	}

}




// semi-private functions

int fra_p_url_init() {

        int rc;


#ifndef NO_PTHREADS
        if( fra_p_pthreads ) {
                rc = pthread_mutex_init( &urls_lock, NULL );
                check( rc, final_cleanup );
        }
#endif

        urls = fra_p_ht_new( 500, sizeof( struct url ), set_url, destroy_url );
        check( urls, urls_lock_cleanup );

        return 0;

urls_lock_cleanup:
#ifndef NO_PTHREADS
        if( fra_p_pthreads ) {
                rc = pthread_mutex_init( &urls_lock, NULL );
                check( rc, final_cleanup );
        }

final_cleanup:
#endif
	return -1;

}

void fra_p_url_deinit() {

        int rc;


	fra_p_lock( &urls_lock, final_cleanup );

        fra_p_ht_free( urls );
	urls = NULL;

	fra_p_unlock( &urls_lock, final_cleanup );

#ifndef NO_PTHREADS
        if( fra_p_pthreads ) {
                rc = pthread_mutex_destroy( &urls_lock );
                check( rc, final_cleanup );
        }
#endif

        return;

final_cleanup:
	return;

}

fra_p_ht_t * fra_p_url_ht_get() {

	return fra_p_ht_new( 500, sizeof( struct url ), set_url, destroy_url );

}

fra_end_t * fra_p_url_to_endpoint( bstring verb, bstring url ) {

	int rc;

	int eq;
	struct url * value;
	fra_end_t * e;


	e = NULL;

	fra_p_lock( &urls_lock, final_cleanup );

	value = (struct url *)fra_p_ht_get( urls, bdata( url ), blength( url ) );

	while( value ) {

		eq = biseq( value->verb, verb );
		check( eq != -1, unlock_cleanup );

		if( eq ) {

			e = value->e;
			break;

		}

		value = value->next;

	}

	fra_p_unlock( &urls_lock, final_cleanup );

	return e;

unlock_cleanup:
	fra_p_unlock( &urls_lock, final_cleanup );

#ifndef NO_PTHREADS
final_cleanup:
#endif
	return NULL;

}




// public functions

int fra_end_url_add( fra_end_t * e, char * verb, char * url ) {

	int rc;

	struct url * value;
	struct url new;
	size_t url_len;


	fra_p_lock( &urls_lock, final_cleanup );

	new.verb = bfromcstr( verb );
	check( new.verb, unlock_cleanup );

	new.e = e;

	url_len = strnlen( url, FRA_CORE_MAX_ENDPOINT_LENGTH );
	check_msg_v(
			url_len < FRA_CORE_MAX_ENDPOINT_LENGTH,
			verb_cleanup,
			"Endpoint url to long or not \\0 terminated in fra_end_url_add().\n"
			"Set FRA_CORE_MAX_ENDPOINT_LENGTH to a higher value in config.h.\n"
			"First 50 chars are: \"%.*s\".",
			50,
			url
		   );

	value = fra_p_ht_get( urls, url, url_len );

	if( value ) {

		while( value->next ) value = value->next;

		value->next = malloc( sizeof( struct url ) );
		check( value->next, verb_cleanup );

		value->next->next = NULL;
		value->next->verb = new.verb;
		value->next->e = e;

	} else {

		rc = fra_p_ht_set( urls, url, &new );
		check( rc == 0, verb_cleanup );

	}

	fra_p_unlock( &urls_lock, final_cleanup );

	return 0;

verb_cleanup:
	bdestroy( new.verb );

unlock_cleanup:
	fra_p_unlock( &urls_lock, final_cleanup );

#ifndef NO_PTHREADS
final_cleanup:
#endif
	return -1;

}
