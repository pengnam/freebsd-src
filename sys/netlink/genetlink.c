/*-
 * SPDX-License-Identifier: BSD-2-Clause-FreeBSD
 *
 * Copyright (c) 2021 Ng Peng Nam Sean
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */
#include <sys/param.h>
#include <sys/kernel.h>
#include <sys/lock.h>
#include <sys/malloc.h>
#include <sys/mbuf.h>
#include <sys/mutex.h>
#include <sys/priv.h>
#include <sys/queue.h>

#include <net/genetlink.h>

#define D(format, ...)                                                        \
	do {                                                                  \
		printf("%-10s " format "\n", __FUNCTION__, ##__VA_ARGS__);    \
	} while (0)

LIST_HEAD(, genl_family) genl_family_list = LIST_HEAD_INITIALIZER(
    genl_family_list);
struct mtx genl_mtx;
#define GENL_LOCK() mtx_lock(&genl_mtx)
#define GENL_UNLOCK() mtx_unlock(&genl_mtx)

MALLOC_DEFINE(M_GENETLINK, "netlink", "Memory used for netlink packets");

static struct genl_family *
genl_find_family_by_id(unsigned int id)
{
	struct genl_family *curfamily;
	LIST_FOREACH (curfamily, &genl_family_list, next) {
		if (curfamily->id == id)
			return curfamily;
	}
	return NULL;
};

static struct genl_family *
genl_find_family_by_name(char *name)
{
	struct genl_family *curfamily;
	LIST_FOREACH (curfamily, &genl_family_list, next) {
		if (strcmp(curfamily->name, name) == 0)
			return curfamily;
	}
	return NULL;
};

static int
genl_get_cmd(
    uint32_t cmd, const struct genl_family *family, struct genl_ops *ops)
{
	for (int i = 0; i < family->n_ops; i++)
		if (family->ops[i].cmd == cmd) {
			ops = &(family->ops[i]);
		}

	return ENOENT;
}

static int
genl_receive_message_dumpit(struct genl_ops *op, struct nlmsghdr *hdr)
{
	// TODO:
	return EOPNOTSUPP;
}

static int
genl_receive_message_doit(struct genl_ops *op, struct nlmsghdr *hdr)
{
	// TODO:
	return EOPNOTSUPP;
}

static int
genl_receive_message_family(
    const struct genl_family *family, struct nlmsghdr *nlh)
{
	struct genl_ops *ops = NULL;
	struct genlmsghdr *genlmsg = nlmsg_data(nlh);
	int hdrlen;

	if (genl_get_cmd(genlmsg->cmd, family, ops))
		return EOPNOTSUPP;

	// TODO: Check permissions

	hdrlen = GENL_HDRLEN + family->hdrsize;
	if (nlh->nlmsg_len < nlmsg_msg_size(hdrlen))
		return EINVAL;

	if ((nlh->nlmsg_flags & NLM_F_DUMP) == NLM_F_DUMP)
		return genl_receive_message_dumpit(ops, nlh);
	else
		return genl_receive_message_doit(ops, nlh);
}

// At least has size of nlmsghdr
static int
genl_receive_message(void *data, struct socket *so)
{
	struct nlmsghdr *nlmsg;
	struct genl_family *curfamily;

	nlmsg = (struct nlmsghdr *)data;
	curfamily = genl_find_family_by_id(nlmsg->nlmsg_type);
	if (curfamily == NULL) {
		D("Family not found");
		return ENOENT;
	}
	// TODO: Check max attribute
	if (nlmsg->nlmsg_len < (NLMSG_HDRLEN + GENL_HDRLEN)) {
		return EBADMSG;
	}

	return genl_receive_message_family(curfamily, nlmsg);
}

int
genl_register_family(struct genl_family *family)
{
	if (genl_find_family_by_name(family->name)) {
		return EEXIST;
	}
	// TODO: Ensure unique ID

	GENL_LOCK();
	LIST_INSERT_HEAD(&genl_family_list, family, next);
	GENL_UNLOCK();

	return 0;
}
int
genl_unregister_family(const struct genl_family *family)
{
	struct genl_family *fam = genl_find_family_by_id(family->id);
	if (fam != NULL) {
		LIST_REMOVE(fam, next);
		free(fam, M_GENETLINK);
		return 0;
	} else {
		return ENOENT;
	}
}

// TODO: Decide which ptr should be returned
void *
genlmsg_put(struct mbuf *m, uint32_t portid, uint32_t seq,
    const struct genl_family *family, int flags, uint8_t cmd)
{

	struct nlmsghdr *nlh;
	struct genlmsghdr *g;

	nlh = nlmsg_put(
	    m, portid, seq, family->id, GENL_HDRLEN + family->hdrsize, flags);
	if (nlh == NULL)
		return nlh;
	g = nlmsg_data(nlh);
	g->cmd = cmd;
	g->version = family->version;
	g->reserved = 0;

	return g;
}

static void
genl_load(void *u __unused)
{
	// TODO: initialize
	nl_register_or_replace_handler(NETLINK_GENERIC, genl_receive_message);
	// TODO: initialize bsd nl
}

static void
genl_unload(void *u __unused)
{
}

SYSINIT(genl_load, SI_SUB_PROTO_DOMAIN, SI_ORDER_THIRD, genl_load, NULL);
SYSINIT(genl_unload, SI_SUB_PROTO_DOMAIN, SI_ORDER_THIRD, genl_unload, NULL);
