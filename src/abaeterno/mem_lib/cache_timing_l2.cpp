// (C) Copyright 2006-2009 Hewlett-Packard Development Company, L.P.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
// THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
// OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
// ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
// OTHER DEALINGS IN THE SOFTWARE.
//

// $Id$

#include "cache_timing_l2.h"
#include "error.h"

namespace Memory {
namespace Timing {

static inline uint32_t _log2(uint32_t num)
{
    uint32_t log2 = 0;

    if (num == 0)
	  throw std::runtime_error("Cannot compute log2(0)");

    while (num > 1)
    {
      num = (num >> 1);
      log2++;
    }

    return log2;
}

static inline void reset_vector(std::vector<uint64_t>& vect)
{
    for (std::vector<uint64_t>::iterator it=vect.begin(); it != vect.end(); it++)
      (*it)=0;
}

L2::L2(const Parameters & p)
 : _nbanks           (p.get<uint32_t>("nbanks", "1")),
  _nsubbanks_per_bank(p.get<uint32_t>("nsubbanks_per_bank", "1")),
  _page_sz           (p.get<uint32_t>("page_size", "1")),
  _line_sz           (p.get<uint32_t>("line_size", "1")),
  _clock_multiplier  (p.get<uint32_t>("clock_multiplier", "1")),
  _t_RC              (p.get<uint32_t>("t_RC", "1")),
  _t_RAS             (p.get<uint32_t>("t_RAS", "1")),
  _t_additional_hit_latency(p.get<uint32_t>("t_additional_hit_latency", "1")),
  _latest_service_time(0),
  _n_bank_conflict   (_nbanks),
  _n_subbank_access  (_nbanks*_nsubbanks_per_bank),
  _bank_latest_access_cycle       (_nbanks,                     0),
  _subbank_latest_access_cycle    (_nbanks*_nsubbanks_per_bank, 0)
{
  if (_line_sz > _page_sz)
    ERROR("Cache line size '",_line_sz,"' must be smaller than the page size '",_page_sz,"'");

  /*
  for (uint32_t i = 0; i < _nbanks*_nsubbanks_per_bank; i++)
  {
    add(".n_subbank_access_"+boost::lexical_cast<std::string>(i)+" ", _n_subbank_access[i]);
  }
  for (uint32_t i = 0; i < _nbanks; i++)
  {
    add(".n_bank_cnflt_"    +boost::lexical_cast<std::string>(i)+" ", _n_bank_conflict[i]);
  } 
  */ // Jung Ho : interesting in general, but not now. 
  
  add("total_contention_cycle ", _total_contention_cycle);
  add("total_latency ", _total_latency);
  clear_metrics();

  _page_sz_log2 = _log2(_page_sz);
  _line_sz_log2 = _log2(_line_sz);
  _nbanks_log2  = _log2(_nbanks);
  _nsubbanks_per_bank_log2 = _log2(_nsubbanks_per_bank);
}

void L2::reset()
{
  //_n_bank_conflict.clear();
  //_n_subbank_access.clear(),
  reset_vector( _bank_latest_access_cycle );
  reset_vector( _subbank_latest_access_cycle );

  _latest_service_time = 0;
}

uint32_t L2::latency(uint64_t tstamp, const Trace& mt, const Access& ma)
{
  uint32_t latency = 0;

  // normalize clock
  if (tstamp % _clock_multiplier != 0)
  {
    latency += _clock_multiplier - (tstamp%_clock_multiplier);
    tstamp  += latency;
  }

  // TODO: how to map an address to a bank is important and must be
  // parameterized.
  uint32_t bank_num = (ma.phys >> _line_sz_log2) % _nbanks;


  // bank conflict
  if (_bank_latest_access_cycle[bank_num] + _clock_multiplier > tstamp)
  {
    _n_bank_conflict[bank_num]++;
    _total_contention_cycle += _bank_latest_access_cycle[bank_num] - tstamp + _clock_multiplier;
    latency                 += _bank_latest_access_cycle[bank_num] - tstamp + _clock_multiplier;
    tstamp                   = _bank_latest_access_cycle[bank_num]          + _clock_multiplier;
  }
  _bank_latest_access_cycle[bank_num] = tstamp;

  uint32_t subbank_num = (bank_num << _nsubbanks_per_bank_log2) +
                         ((ma.phys >> (_page_sz_log2+_nbanks_log2)) % _nsubbanks_per_bank);

  if (_subbank_latest_access_cycle[subbank_num] + _t_RC > tstamp)
  {
    _n_subbank_access[subbank_num]++;
    _total_contention_cycle += _subbank_latest_access_cycle[subbank_num] + _t_RC - tstamp;
    latency                 += _subbank_latest_access_cycle[subbank_num] + _t_RC - tstamp;
    tstamp                   = _subbank_latest_access_cycle[subbank_num] + _t_RC;
  }
  _subbank_latest_access_cycle[subbank_num] = tstamp;
  latency += _t_RAS + _t_additional_hit_latency;

  _total_latency += latency;
  return latency;
}

MainMemory::MainMemory(const Parameters& p):
  _nchannels          (p.get<uint32_t>("nchannels",            "1")),
  _ndimms_per_channel (p.get<uint32_t>("ndimms_per_channel",   "1")),
  _ndevices_per_dimm  (p.get<uint32_t>("ndevices_per_dimm",    "1")),
  _nbanks_per_device  (p.get<uint32_t>("nbanks_per_device",    "1")),
  _page_sz            (p.get<uint32_t>("page_size",            "1")),
  _line_sz            (p.get<uint32_t>("line_size",            "1")),
  _cmd_clk_multiplier (p.get<uint32_t>("cmd_clk_multiplier",   "1")),
  _bank_clk_multiplier(p.get<uint32_t>("bank_clk_multiplier",  "1")),
  _channel_latest_access_cycle(_nchannels, 0),
  _device_latest_access_cycle(_nchannels*_ndevices_per_dimm, 0),
  _dimm_latest_access_cycle(_nchannels*_ndimms_per_channel*_ndevices_per_dimm, 0),
  _bank_latest_access_cycle(_nchannels*_ndimms_per_channel*_ndevices_per_dimm*_nbanks_per_device, 0),
  _total_latency      (0),
  _t_RC               (p.get<uint32_t>("t_RC", "1")),
  _t_RAS              (p.get<uint32_t>("t_RAS", "1")),
  _t_data_transfer    (p.get<uint32_t>("t_data_transfer", "1"))
{
	if (_line_sz > _page_sz)
        ERROR("Cache line size '",_line_sz,"' must be smaller than the page size '",_page_sz,"'");

	add("total_latency ", _total_latency);
	clear_metrics();

	_page_sz_log2            = _log2(_page_sz);
	_line_sz_log2            = _log2(_line_sz);
	_nchannels_log2          = _log2(_nchannels);
	_ndimms_per_channel_log2 = _log2(_ndimms_per_channel);
	_ndevices_per_dimm_log2  = _log2(_ndevices_per_dimm);
	_nbanks_per_device_log2  = _log2(_nbanks_per_device);
}

void MainMemory::reset()
{
  reset_vector( _channel_latest_access_cycle );
  reset_vector( _device_latest_access_cycle );
  reset_vector( _dimm_latest_access_cycle );
  reset_vector( _bank_latest_access_cycle );
}

uint32_t MainMemory::latency(uint64_t tstamp,const Trace& mt,const Access& ma)
{
  uint32_t latency = 0;

  // normalize clock to _cmd_clk_multiplier
  if (tstamp % _cmd_clk_multiplier != 0)
  {
    latency += _cmd_clk_multiplier - (tstamp%_cmd_clk_multiplier);
    tstamp  += latency;
  }

  // address interleaving
  // page  |  column in a page  |  bank  |  dimm  |  device  |  channel  |  cache line
  uint32_t channel_num = (ma.phys >> _line_sz_log2) % _nchannels;
  uint32_t device_num  = (ma.phys >> _line_sz_log2) %
                         (_ndevices_per_dimm*_nchannels);
  uint32_t dimm_num    = (ma.phys >> _line_sz_log2) %
                         (_ndimms_per_channel*_ndevices_per_dimm*_nchannels);
  uint32_t bank_num    = (ma.phys >> _line_sz_log2) %
                         (_nbanks_per_device*_ndimms_per_channel*_ndevices_per_dimm*_nchannels);

  // channel command path conflict
  if (_channel_latest_access_cycle[channel_num] + _cmd_clk_multiplier > tstamp)
  {
    latency += _channel_latest_access_cycle[channel_num] - tstamp + _cmd_clk_multiplier;
    tstamp   = _channel_latest_access_cycle[channel_num]          + _cmd_clk_multiplier;
  }
  _channel_latest_access_cycle[channel_num] = tstamp;

  // device level conflict (actually data transfer time)
  if (_device_latest_access_cycle[device_num] + _t_data_transfer > tstamp)
  {
    latency += _device_latest_access_cycle[device_num] - tstamp + _t_data_transfer;
    tstamp   = _device_latest_access_cycle[device_num]          + _t_data_transfer;
  }
  _device_latest_access_cycle[device_num] = tstamp;

  // dimm level conflict (actually tRRD)
  if (_dimm_latest_access_cycle[dimm_num] + _bank_clk_multiplier > tstamp)
  {
    latency += _dimm_latest_access_cycle[dimm_num] - tstamp + _bank_clk_multiplier;
    tstamp   = _dimm_latest_access_cycle[dimm_num]          + _bank_clk_multiplier;
  }
  _dimm_latest_access_cycle[dimm_num] = tstamp;

  // bank level conflict (actually tRC)
  if (_bank_latest_access_cycle[bank_num] + _t_RC > tstamp)
  {
    latency += _bank_latest_access_cycle[bank_num] - tstamp + _t_RC;
    tstamp   = _bank_latest_access_cycle[bank_num]          + _t_RC;
  }
  _bank_latest_access_cycle[bank_num] = tstamp;

  latency += _t_RAS;
  _total_latency += latency;
  return latency;
}

}
}
