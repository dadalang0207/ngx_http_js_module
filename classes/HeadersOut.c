
// Nginx.HeadersOut class

#include <ngx_config.h>
#include <ngx_core.h>
#include <ngx_http.h>
#include <nginx.h>

#include <jsapi.h>
#include <assert.h>

#include "../ngx_http_js_module.h"
#include "../strings_util.h"
#include "Request.h"

#include "../macroses.h"

//#define unless(a) if(!(a))
#define JS_HEADER_IN_ROOT_NAME      "Nginx.HeadersOut instance"


JSObject *ngx_http_js__nginx_headers_out__prototype;
JSClass ngx_http_js__nginx_headers_out__class;
static JSClass* private_class = &ngx_http_js__nginx_headers_out__class;

static ngx_table_elt_t *
search_headers_out(ngx_http_request_t *r, char *name, u_int len);


JSObject *
ngx_http_js__nginx_headers_out__wrap(JSContext *cx, ngx_http_request_t *r)
{
	TRACE();
	JSObject                  *headers;
	ngx_http_js_ctx_t         *ctx;
	
	if (!(ctx = ngx_http_get_module_ctx(r, ngx_http_js_module)))
		ngx_http_js__nginx_request__wrap(cx, r);
	
	if (ctx->js_headers_out)
		return ctx->js_headers_out;
	
	headers = JS_NewObject(cx, &ngx_http_js__nginx_headers_out__class, ngx_http_js__nginx_headers_out__prototype, NULL);
	if (!headers)
	{
		JS_ReportOutOfMemory(cx);
		return NULL;
	}
	
	if (!JS_AddNamedRoot(cx, &ctx->js_headers_out, JS_HEADER_IN_ROOT_NAME))
	{
		JS_ReportError(cx, "Can`t add new root %s", JS_HEADER_IN_ROOT_NAME);
		return NULL;
	}
	
	JS_SetPrivate(cx, headers, r);
	
	ctx->js_headers_out = headers;
	
	return headers;
}


void
ngx_http_js__nginx_headers_out__cleanup(JSContext *cx, ngx_http_request_t *r, ngx_http_js_ctx_t *ctx)
{
	TRACE();
	
	assert(ctx);
	
	if (!ctx->js_headers_out)
		return;
	
	if (!JS_RemoveRoot(cx, &ctx->js_headers_out))
		JS_ReportError(cx, "Can`t remove cleaned up root %s", JS_HEADER_IN_ROOT_NAME);
	
	JS_SetPrivate(cx, ctx->js_headers_out, NULL);
	ctx->js_headers_out = NULL;
}


static JSBool
constructor(JSContext *cx, JSObject *this, uintN argc, jsval *argv, jsval *rval)
{
	TRACE();
	return JS_TRUE;
}


// enum propid { HEADER_LENGTH };


static JSBool
getProperty(JSContext *cx, JSObject *this, jsval id, jsval *vp)
{
	ngx_http_request_t         *r;
	char                       *name;
	ngx_table_elt_t            *header;
	
	TRACE();
	GET_PRIVATE(r);
	
	if (JSVAL_IS_STRING(id) && (name = JS_GetStringBytes(JSVAL_TO_STRING(id))) != NULL)
	{
		// if (!strcmp(member_name, "constructor"))
		// LOG("getProperty: %s", name);
		
		header = search_headers_out(r, name, 0);
		
		if (header)
			*vp = STRING_TO_JSVAL(JS_NewStringCopyN(cx, (char *) header->value.data, header->value.len));
		// else
		// 	LOG("getProperty: %s was not found", name);
	}
	
	// *vp = STRING_TO_JSVAL(JS_NewStringCopyZ(cx, "not set"));
	
	return JS_TRUE;
}


static JSBool
setProperty(JSContext *cx, JSObject *this, jsval id, jsval *vp)
{
	ngx_http_request_t         *r;
	ngx_table_elt_t            *header;
	char                       *key;
	size_t                      key_len;
	JSString                   *key_jsstr, *value_jsstr;
	
	TRACE();
	GET_PRIVATE(r);
	
	// E(JSVAL_IS_STRING(id), "Nginx.Request#[]= takes a key:String and a value of a key relational type");
	if (JSVAL_IS_STRING(id))
	{
		key_jsstr = JSVAL_TO_STRING(id);
		E(js_str2c_str(cx, key_jsstr, r->pool, &key, &key_len), "Can`t js_str2c_str(key_jsstr)");
		E(value_jsstr = JS_ValueToString(cx, *vp), "Can`t JS_ValueToString()");
		
		// LOG("setProperty: %s (%u)", key, (int)key_len);
		
		header = search_headers_out(r, key, key_len);
		if (header)
		{
			header->key.data = (u_char*)key;
			header->key.len = key_len;
			E(js_str2ngx_str(cx, value_jsstr, r->pool, &header->value, 0), "Can`t js_str2ngx_str(value_jsstr)");
			
			return JS_TRUE;
		}
		
		
		header = ngx_list_push(&r->headers_out.headers);
		if (header)
		{
			header->hash = 1;
		
			header->key.data = (u_char*)key;
			header->key.len = key_len;
			E(js_str2ngx_str(cx, value_jsstr, r->pool, &header->value, 0), "Can`t js_str2ngx_str(value_jsstr)");
			
			if (NCASE_COMPARE(header->key, "Content-Length"))
			{
				E(JSVAL_IS_INT(*vp), "the Content-Length value must be an Integer");
				r->headers_out.content_length_n = (off_t) JSVAL_TO_INT(*vp);
				r->headers_out.content_length = header;
				
				return JS_TRUE;
			}
		}
		else
			THROW("Can`t ngx_list_push()")
	}
	
	
	return JS_TRUE;
}

static JSBool
delProperty(JSContext *cx, JSObject *this, jsval id, jsval *vp)
{
	TRACE();
	return JS_TRUE;
}

JSPropertySpec ngx_http_js__nginx_headers_out__props[] =
{
	// {"uri",      REQUEST_URI,          JSPROP_READONLY,   NULL, NULL},
	{0, 0, 0, NULL, NULL}
};


JSFunctionSpec ngx_http_js__nginx_headers_out__funcs[] = {
    // {"empty",       method_empty,          1, 0, 0},
    {0, NULL, 0, 0, 0}
};

JSClass ngx_http_js__nginx_headers_out__class =
{
	"HeadersOut",
	JSCLASS_HAS_PRIVATE,
	JS_PropertyStub, delProperty, getProperty, setProperty,
	JS_EnumerateStub, JS_ResolveStub, JS_ConvertStub, JS_FinalizeStub,
	JSCLASS_NO_OPTIONAL_MEMBERS
};

JSBool
ngx_http_js__nginx_headers_out__init(JSContext *cx)
{
	JSObject    *nginxobj;
	JSObject    *global;
	jsval        vp;
	
	TRACE();
	global = JS_GetGlobalObject(cx);
	
	E(JS_GetProperty(cx, global, "Nginx", &vp), "global.Nginx is undefined or is not a function");
	nginxobj = JSVAL_TO_OBJECT(vp);
	
	ngx_http_js__nginx_headers_out__prototype = JS_InitClass(cx, nginxobj, NULL, &ngx_http_js__nginx_headers_out__class,  constructor, 0,
		ngx_http_js__nginx_headers_out__props, ngx_http_js__nginx_headers_out__funcs,  NULL, NULL);
	E(ngx_http_js__nginx_headers_out__prototype, "Can`t JS_InitClass(Nginx.HeadersOut)");
	
	return JS_TRUE;
}


static ngx_table_elt_t *
search_headers_out(ngx_http_request_t *r, char *name, u_int len)
{
	ngx_list_part_t            *part;
	ngx_table_elt_t            *h;
	ngx_uint_t                  i;
	
	TRACE();
	assert(r);
	assert(name);
	
	if (len == 0)
	{
		len = strlen(name);
		if (len == 0)
			return NULL;
	}
		
	// look in all headers
	
	part = &r->headers_out.headers.part;
	h = part->elts;
	
	for (i = 0; /* void */ ; i++)
	{
		if (i >= part->nelts)
		{
			if (part->next == NULL)
				break;
			
			part = part->next;
			h = part->elts;
			i = 0;
		}
		
		// LOG("%s", h[i].key.data);
		if (len != h[i].key.len || ngx_strcasecmp((u_char *) name, h[i].key.data) != 0)
			continue;
		
		return &h[i];
	}
	
	return NULL;
}



// check this from time to time
// http://emiller.info/lxr/http/source/http/ngx_http_request.h#L252
// 
// typedef struct
// {
//     ngx_list_t                        headers;
//     
//     ngx_uint_t                        status;
//     ngx_str_t                         status_line;
//     
//     ngx_table_elt_t                  *server;
//     ngx_table_elt_t                  *date;
//     ngx_table_elt_t                  *content_length;
//     ngx_table_elt_t                  *content_encoding;
//     ngx_table_elt_t                  *location;
//     ngx_table_elt_t                  *refresh;
//     ngx_table_elt_t                  *last_modified;
//     ngx_table_elt_t                  *content_range;
//     ngx_table_elt_t                  *accept_ranges;
//     ngx_table_elt_t                  *www_authenticate;
//     ngx_table_elt_t                  *expires;
//     ngx_table_elt_t                  *etag;
//     
//     ngx_str_t                        *override_charset;
//     
//     size_t                            content_type_len;
//     ngx_str_t                         content_type;
//     ngx_str_t                         charset;
//     
//     ngx_array_t                       cache_control;
//     
//     off_t                             content_length_n;
//     time_t                            date_time;
//     time_t                            last_modified_time;
// }
// ngx_http_headers_out_t;
