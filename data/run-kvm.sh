#!/bin/sh

exec kvm -m 256 -smp 1 -drive file=karmic64.img -cdrom cotson-guest-tools.iso "$@"
