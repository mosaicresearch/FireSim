#ifndef CLICK_STATICCOUNTER_HH
#define CLICK_STATICCOUNTER_HH
#include <click/element.hh>
#include <click/string.hh>
#include <map>
CLICK_DECLS

class StaticCounter : public Element { public:

    StaticCounter();
    ~StaticCounter();

    const char *class_name() const		{ return "StaticCounter"; }
    const char *port_count() const		{ return "0-1/0-1"; }
    const char *processing() const		{ return PUSH; }

    int configure(Vector<String> &, ErrorHandler *);
    void add_handlers();

    void push(int, Packet *);

  private:
	String _table;
	String _rule;

	static std::map<String,int> _nat;
	static std::map<String,int> _mangle;
	static std::map<String,int> _filter;

    static String unused_handler(Element *, void *);
};

CLICK_ENDDECLS
#endif
