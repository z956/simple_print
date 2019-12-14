SimplePrint
===========
This is a practice proejct for printf-like functions.

Usage
-----
Include header ``simple_print.h``.

.. code:: c++

    simple_print::printf("{}", 5); // print the integer 5

* Arguments are enclosed in ``{}``
* Use ``{{`` and ``}}`` to print ``{`` and ``}`` respectively

Custom type is supported

.. code:: c++

    struct custom_type
    {
        int i;
    };

    template<>
    struct format_type<custom_type>
    {
        static std::string format(const custom_type& t)
        {
            return std::string("custom_type: ") + std::to_string(i);
        }
    };

    custom_type c;
    simple_print::printf("{}", c);

TODO
----
* Support placeholder
* Support unicode
* Support different type of result
