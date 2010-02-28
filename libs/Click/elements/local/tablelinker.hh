/**
 * @file tablelinker.hh
 * @author Nico Van Looy (s0061909) & Jens De Wit (s0061864)
 * @brief Bachelor project of University of Antwerp - 2008-2009
 */

#ifndef TABLELINKER_HH_
#define TABLELINKER_HH_

#include <click/element.hh>
#include <vector>

CLICK_DECLS

class TableLinker : public Element { public:

	/**
	 * Constructor
	 */
	TableLinker();

	/**
	 * Destructor
	 */
    ~TableLinker();

    /**
     * Return the element's class name.
     * @return the class name
     */
    const char *class_name() const		{ return "TableLinker"; }

    /**
     * Returns the element's port count specifier.
     * Click extracts port count specifiers from the source for use by tools.
     * For Click to find a port count specifier, the function definition must appear inline,
     * on a single line, inside the element class's declaration, and must return a C string constant.
     * It should also have public accessibility.
     * @return C string constant as an indication for the number of input and output ports of this element
     */
    const char *port_count() const		{ return "1/0"; }

    /**
     * Return the element's processing specifier.
     * @return C string constant as an indication for the processing mode.
     */
    const char *processing() const		{ return PUSH; }

    /**
     * Parse the element's configuration arguments.
     * @param conf The configuration arguments
     * @param errh The error handler
     */
    int configure(Vector<String> &conf, ErrorHandler *errh);

    /**
     * Add read and write handlers
     */
    void add_handlers();

    /**
     * Notification
     */
    void onFromEvent(Element*);

    /**
     * Notification
     */
    void onChainEvent(String chain);

    /**
     * Push packet p from an certain input port to an push input port.
     */
    void push(int, Packet *);

  private:
	  Element* _accept;
	  Element* _from;
	  Element* _manglePrerouting;
	  Element* _mangleInput;
	  Element* _mangleForward;
	  Element* _mangleOutput;
	  Element* _manglePostrouting;
	  Element* _natPrerouting;
	  Element* _natPostrouting;
	  Element* _natOutput;
	  Element* _filterInput;
	  Element* _filterForward;
	  Element* _filterOutput;
	  String _chain;
	  static int from_handler(const String &in_str, Element *e, void *thunk, ErrorHandler *errh);
	  static int chain_handler(const String &in_str, Element *e, void *thunk, ErrorHandler *errh);
};

#endif /* TABLELINKER_HH_ */
