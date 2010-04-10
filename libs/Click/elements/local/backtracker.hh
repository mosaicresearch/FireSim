#ifndef CLICK_BACKTRACKER_HH
#define CLICK_BACKTRACKER_HH
#include <click/element.hh>
CLICK_DECLS

class Backtracker : public Element { public:

	/**
	 * Constructor
	 */
	Backtracker();

	/**
	 * Destructor
	 */
    ~Backtracker();

    /**
     * Return the element's class name.
     * @return the class name
     */
    const char *class_name() const		{ return "Backtracker"; }

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
     * Push packet p from an certain input port to an push input port.
     */
    void push(int, Packet *);

};

CLICK_ENDDECLS
#endif // CLICK_BACKTRACKER_HH
