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
#ifndef _LINUX_NETLINK_H
#define _LINUX_NETLINK_H

#define AF_NETLINK 38
#define PF_NETLINK 38

#include <sys/types.h>
#include <sys/socket.h>
#include <linux/if_link.h>

#define NETLINK_ROUTE 0	    /* Routing/device hook				*/
#define NETLINK_UNUSED 1    /* Unused number				*/
#define NETLINK_USERSOCK 2  /* Reserved for user mode socket protocols 	*/
#define NETLINK_FIREWALL 3  /* Unused number, formerly ip_queue		*/
#define NETLINK_SOCK_DIAG 4 /* socket monitoring \
			     */
#define NETLINK_NFLOG 5	    /* netfilter/iptables ULOG */
#define NETLINK_XFRM 6	    /* ipsec */
#define NETLINK_SELINUX 7   /* SELinux event notifications */
#define NETLINK_ISCSI 8	    /* Open-iSCSI */
#define NETLINK_AUDIT 9	    /* auditing */
#define NETLINK_FIB_LOOKUP 10
#define NETLINK_CONNECTOR 11
#define NETLINK_NETFILTER 12 /* netfilter subsystem */
#define NETLINK_IP6_FW 13
#define NETLINK_DNRTMSG 14	  /* DECnet routing messages */
#define NETLINK_KOBJECT_UEVENT 15 /* Kernel messages to userspace */
#define NETLINK_GENERIC 16

struct sockaddr_nl {
	uint8_t nl_len;	       /* FreeBSD SPECIFIC */
	sa_family_t nl_family; /* AF_NETLINK */
	uint16_t nl_pad;       /* zero */
	uint32_t nl_pid;       /* port ID */
	uint32_t nl_groups;    /* multicast groups mask */
};

struct nlmsghdr {
	uint32_t nlmsg_len;   /* Length of message including header */
	uint16_t nlmsg_type;  /* Message content */
	uint16_t nlmsg_flags; /* Additional flags */
	uint32_t nlmsg_seq;   /* Sequence number */
	uint32_t nlmsg_pid;   /* Sending process port ID */
};
struct nlmsgerr {
	int error;
	struct nlmsghdr msg;
};

// COPIED FROM LINUX VERSION
/* Flags values */
#define NLM_F_REQUEST 0x01   /* It is request message. 	*/
#define NLM_F_MULTI 0x02     /* Multipart message, terminated by NLMSG_DONE */
#define NLM_F_ACK 0x04	     /* Reply with ack, with zero or error code */
#define NLM_F_ECHO 0x08	     /* Echo this request 		*/
#define NLM_F_DUMP_INTR 0x10 /* Dump was inconsistent due to sequence change \
			      */
#define NLM_F_DUMP_FILTERED 0x20 /* Dump was filtered as requested */

/* Modifiers to GET request */
#define NLM_F_ROOT 0x100   /* specify tree	root	*/
#define NLM_F_MATCH 0x200  /* return all matching	*/
#define NLM_F_ATOMIC 0x400 /* atomic GET		*/
#define NLM_F_DUMP (NLM_F_ROOT | NLM_F_MATCH)

/* Modifiers to NEW request */
#define NLM_F_REPLACE 0x100 /* Override existing		*/
#define NLM_F_EXCL 0x200    /* Do not touch, if it exists	*/
#define NLM_F_CREATE 0x400  /* Create, if it does not exist	*/
#define NLM_F_APPEND 0x800  /* Add to end of list		*/

/* Modifiers to DELETE request */
#define NLM_F_NONREC 0x100 /* Do not delete recursively	*/

/* Flags for ACK message */
#define NLM_F_CAPPED 0x100   /* request was capped */
#define NLM_F_ACK_TLVS 0x200 /* extended ACK TVLs were included */

#define NLMSG_NOOP 0x1	  /* Nothing.		*/
#define NLMSG_ERROR 0x2	  /* Error		*/
#define NLMSG_DONE 0x3	  /* End of a dump	*/
#define NLMSG_OVERRUN 0x4 /* Data lost		*/

#define NLMSG_MIN_TYPE 0x10 /* < 0x10: reserved control messages */

#define NLMSG_ALIGNTO 4U
#define NLMSG_ALIGN(len) (((len) + NLMSG_ALIGNTO - 1) & ~(NLMSG_ALIGNTO - 1))
#define NLMSG_HDRLEN ((int)NLMSG_ALIGN(sizeof(struct nlmsghdr)))

// Socket options
#define NETLINK_ADD_MEMBERSHIP 1
#define NETLINK_DROP_MEMBERSHIP 2
// Others currently supported
#if 0
#define NETLINK_PKTINFO 3
#define NETLINK_BROADCAST_ERROR 4
#define NETLINK_NO_ENOBUFS 5
#define NETLINK_RX_RING 6
#define NETLINK_TX_RING 7
#define NETLINK_LISTEN_ALL_NSID 8
#define NETLINK_LIST_MEMBERSHIPS 9
#define NETLINK_CAP_ACK 10
#define NETLINK_EXT_ACK 11
#define NETLINK_GET_STRICT_CHK 12
#endif

/*---- start nl attributes----*/
struct nlattr {
	uint16_t nla_len;
	uint16_t nla_type;
};

/*
 * nla_type (16 bits)
 * +---+---+-------------------------------+
 * | N | O | Attribute Type                |
 * +---+---+-------------------------------+
 * N := Carries nested attributes
 * O := Payload stored in network byte order
 *
 * Note: The N and O flag are mutually exclusive.
 */
#define NLA_F_NESTED (1 << 15)
#define NLA_F_NET_BYTEORDER (1 << 14)
#define NLA_TYPE_MASK ~(NLA_F_NESTED | NLA_F_NET_BYTEORDER)

#define NLA_ALIGNTO 4
#define NLA_ALIGN(len) (((len) + NLA_ALIGNTO - 1) & ~(NLA_ALIGNTO - 1))
#define NLA_HDRLEN ((int)NLA_ALIGN(sizeof(struct nlattr)))

#endif /*linux_netlink_h*/
