#ifndef _NGX_STREAM_UPSTREAM_CHECK_MODULE_H_INCLUDED_
#define _NGX_STREAM_UPSTREAM_CHECK_MODULE_H_INCLUDED_


#include <ngx_config.h>
#include <ngx_core.h>

ngx_uint_t ngx_stream_upstream_check_add_peer(ngx_conf_t *cf,
    ngx_stream_upstream_srv_conf_t *us, ngx_addr_t *peer);

ngx_uint_t ngx_stream_upstream_check_peer_down(ngx_uint_t index);

void ngx_stream_upstream_check_get_peer(ngx_uint_t index);
void ngx_stream_upstream_check_free_peer(ngx_uint_t index);


#endif //_NGX_STREAM_UPSTREAM_CHECK_MODULE_H_INCLUDED_

