/*
 * acore_input.c
 *
 *  Created on: Aug 31, 2022
 *      Author: amneiht
 */

#include <acore/ui.h>
#include <acore.h>


static void sc_callback(pj_scanner *scanner) {
	(void) scanner;
	pj_scan_get_char(scanner);
}

pj_bool_t acore_ui_simple_parse(pj_str_t *data, acore_ui_simple_opt *opt,
		int opt_len) {
	pj_cis_buf_t cs_buff;
	pj_cis_t *cis = alloca(sizeof(pj_cis_t));
	pj_cis_buf_init(&cs_buff);
	pj_cis_init(&cs_buff, cis);
	pj_cis_add_str(cis, " ");
	pj_cis_invert(cis);
	pj_cis_del_str(cis, "\"\'\%");
	pj_scanner *scanner = alloca(sizeof(pj_scanner));
	pj_scan_init(scanner, data->ptr, data->slen, PJ_SCAN_AUTOSKIP_WS,
			sc_callback);

	int count = opt_len;
	pj_str_t out;
	// set all value is empty
	for (int i = 0; i < opt_len; i++) {
		opt[i].value.slen = 0;
	}
	while (count > 0 && !pj_scan_is_eof(scanner)) {
		pj_scan_get(scanner, cis, &out);
		for (int i = 0; i < opt_len; i++) {
			if (pj_strcmp(&opt[i].name, &out) == 0) {
				if (opt[i].value.slen == 0) {
					// get value
					if (scanner->curptr[0] == '\''
							|| scanner->curptr[0] == '\"') {
						pj_scan_get_quotes(scanner, "\"'", "\"'", 2, &out);
					} else
						pj_scan_get(scanner, cis, &out);
					count--;
					opt[i].value = out;
					break;
				}
			}
		}
	}
	pj_scan_fini(scanner);
	return count == 0;
}
