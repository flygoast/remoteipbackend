/*
    PowerDNS backend module.
    It used to give a DNS response whose content is the IP address from 
    which the DNS request came.

    Copyright (c) 2014, FengGu <flygoast@126.com>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License version 2
    as published by the Free Software Foundation

    Additionally, the license of this program contains a special
    exception which allows to distribute the program in binary form when
    it is linked against OpenSSL.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
*/


#include "pdns/utility.hh"
#include "pdns/dnsbackend.hh"
#include "pdns/dns.hh"
#include "pdns/dnsbackend.hh"
#include "pdns/dnspacket.hh"
#include "pdns/pdnsexception.hh"
#include "pdns/logger.hh"
#include <boost/algorithm/string.hpp>


class RemoteIPBackend : public DNSBackend
{
public:
    RemoteIPBackend(const string &suffix="") 
    {
        setArgPrefix("remoteip"+suffix);
        d_ourname=getArg("domain");
    }

    bool list(const string &target, int id, bool include_disabled) {
        return false; // we don't support AXFR
    }

    void lookup(const QType &type, const string &qdomain, DNSPacket *p, int zoneId)
    {
        if((type.getCode()!=QType::ANY && type.getCode()!=QType::A) || !pdns_iequals(qdomain, d_ourname))
            d_answer=""; // no answer
        else {
            d_answer=p->getRemote();
        }

    }

    bool get(DNSResourceRecord &rr)
    {
        if(!d_answer.empty()) {
            rr.qname=d_ourname; // fill in details
            rr.qtype=QType::A;  // A record
            rr.ttl=0;           // no cache
            rr.auth = 1;
            rr.content=d_answer;

            d_answer="";        // this was the last answer

            return true;
        }

        return false;           // no more data
    }

private:
    string d_answer;
    string d_ourname;
};


class RemoteIPFactory : public BackendFactory
{
public:
    RemoteIPFactory() : BackendFactory("remoteip") {}

    void declareArguments(const string &suffix="")
    {
        declare(suffix,"domain","Domain which is to be remote IP address.","");
    }

    DNSBackend *make(const string &suffix="")
    {
        return new RemoteIPBackend(suffix);
    }
};


class RemoteIPLoader
{
public:
    RemoteIPLoader()
    {
        BackendMakers().report(new RemoteIPFactory);

        L<<Logger::Info<<" [RemoteIPBackend] This is the remoteipbackend version "VERSION" ("__DATE__", "__TIME__") reporting"<<endl;
    }  
};


static RemoteIPLoader remoteipLoader;
