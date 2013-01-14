#!/bin/sh
## we call tracer_app twice -- note the different parameter
## We have previously subscribed to function value 1234 
## (in tracer-example.cpp) so our tracer example gets called. 
## As we are using no_timing only the functional callback gets invoked

xget /tmp/tracer_app /tmp/tracer_app
chmod +x /tmp/tracer_app

## First call to the custom tracer (in functional mode)
/tmp/tracer_app 1234 2 3

## Call "10" is reserved for the selective sampler
## cotson_tracer is part of the guest tools, preinstalled in the BSD
cotson_tracer 10 1 0   ## switch to timing

## Second call to the custom tracer (in timing mode)
/tmp/tracer_app 1234 4 5

## Call "10" is reserved for the selective sampler
## cotson_tracer is part of the guest tools, preinstalled in the BSD
cotson_tracer 10 1 1  ## back to functional

touch terminate; xput terminate terminate
