#!/usr/bin/env lua
-- (C) Copyright 2009 Hewlett-Packard Development Company, L.P.
--
-- THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
-- IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
-- FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
-- THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
-- OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
-- ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
-- OTHER DEALINGS IN THE SOFTWARE.
--

-- $Id$

dofile(arg[0]:gsub("(.+)/.+","%1/defaults.lua"))

use_bsd=use_bsd_helper
use_hdd=use_hdd_helper
set_journal=set_journal_helper
set_quantum=set_quantum_helper
send_keyboard=send_keyboard_helper

if type(simnow)~='table' or type(simnow.commands)~='function' then
	error("missing simnow.commands function")
end

simnow.commands()
if type(simnow)=='table' and type(simnow.xcommands)=='function' then
    simnow.xcommands()
end
