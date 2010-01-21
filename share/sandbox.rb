#!/usr/bin/ruby -w
# (C) Copyright 2009 Hewlett-Packard Development Company, L.P.
#
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
# THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR
# OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
# ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
# OTHER DEALINGS IN THE SOFTWARE.
#

# $Id$

require 'location'
require 'lua_scripts'
require 'options'
require 'fileutils'

class Sandbox < Location
  private
  def create
    sandbox_src=$here.from_root('sandbox')
    debug2 "creating sandbox"
    debug2 "  from #{sandbox_src}"
    debug2 "  into #{@root}"

    raise CotsonError.new(:SandboxCreate, :sandbox => $here, :source => sandbox_src, :destination=>@root.to_str ) if !File.exists?(sandbox_src) || File.exists?(@root)
    
    FileUtils.mkdir_p File.dirname(@root)
    FileUtils.cp_r sandbox_src, @root, :preserve => true
  end   

  def install_abaeterno_in
    debug2 "installing abaeterno.in"
    File.open(data('abaeterno.in'),"w") do |f|
      f << "root='#{$here}'\n"
      f << @lua_defs.to_str
    end
  end

  def install_simnow_pre
    debug2 "installing simnow.pre"
    File.open(data('simnow.pre'),"w") do |f|
      f << @lua_defs.simnow_pre
    end
  end

  def install_simnow_pre_cluster
    debug2 "installing simnow.pre (for cluster node)"
    File.open(data('simnow.pre'),"w") do |f|
      f << @lua_defs.simnow_pre_cluster
    end
  end

  def install_user_script
    debug2 "installing user script"
    FileUtils.cp_r(@opts[:user_script], data('user_script'))
  end

  def install_cluster_sh
    debug2 "installing cluster.sh"
    
    File.open(data('cluster.sh'),"w") do |f|
      f.puts "sudo /sbin/cotson_node_config #{@opts[:NODE]} #{@opts[:TOTAL]} | tee node_config.log 2>&1"
      f.puts "xput node_config.log #{@root}/data/node_config.log"
      f.puts "xget ../data/user_script c"
      f.puts "chmod 0755 ./c"
      f.puts "./c #{@opts[:NODE]} #{@opts[:TOTAL]} | tee stdout.log 2>&1"
      f.puts "xput stdout.log #{@root}/data/stdout.log"
      s_id=0
      @opts[:subscribe_result].each do |s|
        f.puts "xput #{s.chomp} #{@root}/data/res_#{s_id}.#{File.basename(s)}" 
        s_id += 1
      end
      f.puts "touch l"
      f.puts "xput l terminate"
    end
  end

  def install_simnow_dir
    simnow_dir=@opts[:simnow_dir]
    link=@opts[:simnow_dir_link]
    debug2 "installing simnow_dir"
    debug2 " from #{simnow_dir}"
    debug2 " using #{link ? 'link' : 'copy'}"
    %w{ simnow productfile linuxlibs analyzers libs Images icons }.each do |x|
      y=File.join simnow_dir, x
      if !File.exists?(y)
        raise CotsonError.new(:SandboxInstallSimNowDir, :simnow_location=>simnow_dir, :component=>x, :link =>link, :sandbox=>$here)
      end
      link ? FileUtils.ln_sf(y,share) : FileUtils.cp_r(y,share)
    end
  end

  def install_abaeterno_so
    f=File.expand_path(@opts[:abaeterno_so])
    aaso = share('abaeterno.so')
    link=@opts[:abaeterno_so_link]
    raise CotsonError.new(:SandboxInstallAbaeterno, :source=>f, :link=>link, :destination=>aaso, :sandbox=>"#{$here}") if !File.exists?(f)
    debug2 "installing abaeterno.so"
    debug2 " from #{f}"
    debug2 " using #{link ? 'link' : 'copy'}"
    link ? FileUtils.ln_sf(f,aaso) : FileUtils.cp_r(f, aaso)
  end

  def install_injector
    f=@opts[:injector]
    inj = share('injector')
    link=@opts[:injector_link]
    debug2 "installing aaInjector"
    debug2 " from #{f}"
    debug2 " using #{link ? 'link' : 'copy'}"
    link ? FileUtils.ln_sf(f,inj) : FileUtils.cp_r(f, inj)
  end

  def install_mediator
    f=File.expand_path(@opts[:mediator_file])
    med = share('mediator')
    link=@opts[:mediator_link]
    debug2 "installing mediator"
    debug2 " from #{f}"
    debug2 " using #{link ? 'link' : 'copy'}"
    link ? FileUtils.ln_sf(f,med) : FileUtils.cp_r(f, med)
  end

  def install_bsd
    f=File.expand_path(@opts[:bsd])
    bsd=data('node.bsd')
    link=@opts[:bsd_link]
    debug2 "installing node.bsd"
    debug2 " from #{f}"
    debug2 " using #{link ? 'link' : 'copy'}"
    link ? FileUtils.ln_sf(f,bsd) : FileUtils.cp_r(f, bsd)
  end

  def install_hdd
    f=File.expand_path(@opts[:hdd])
    hdd=data('node.hdd')
    link=@opts[:hdd_link]
    debug2 "installing node.hdd"
    debug2 " from #{f}"
    debug2 " using #{link ? 'link' : 'copy'}"
    link ? FileUtils.ln_sf(f,hdd) : FileUtils.cp_r(f, hdd)
  end

  def install_config
    debug2 "installing config"
    @opts.save(data('config'))
  end

  def install_db
    debug2 "installing database"
    File.open(data('database'),'w' ) { |x| YAML.dump(Hash.new, x ) }
  end
  
  def install_test_data
    files_to_copy = []
    [
      'filters',
      'abaeterno_output*',
      'injector_output*',
      'simnow_output',
      'disasm',
      'simpoint_profile.gz',
      'disksim_*',
      'atlas10k.*',
      'statdefs',
      'disktrace_*',
    ].each do |x|
      files_to_copy+=@opts[:test].files(x)
    end
    FileUtils.cp files_to_copy.flatten, data
  end

  public
  def initialize(ld,opts,name=nil)
    @lua_defs=ld
    @opts=opts
    super(name ? name : Location.tempfile(opts[:runid]))
  end

  def install(*args)
    create
    args << :config << :db
    args.each { |x| self.send('install_'+x.id2name) }
  end

  def read_config
    debug2 "reading config"
    @opts=Options.new(nil)
    @opts.load_from_file(data('config'))
  end

  def run(*args)
    command_name=args.shift
    command=bin(command_name)
    raise CotsonError.new(:SandboxRun,:sandbox=>$here, :command=>command_name, :script_location=>command, :arguments => "#{args.join(' ')}") if !File.exists?(command)
    debug2 "running #{command} #{args.join(' ')} (and printing)"
    Execute.run_and_print("#{command} #{args.join(' ')}") 
    debug2 "running #{command} ended"
  end

  def run_output(*args)
    command_name=args.shift
    command=bin(command_name)
    raise CotsonError.new(:SandboxRunOutput,:sandbox => $here, :command=>command_name, :script_location=>command, :arguments => "#{args.join(' ')}") if !File.exists?(command)
    debug2 "running #{command} #{args.join(' ')}"
    a=Execute.run("#{command} #{args.join(' ')}")
    debug2 "running #{command} ended" 
    [a[:status],a[:output]]
  end

  def run_detached(*args)
    command_name=args.shift
    command=bin(command_name)
    raise CotsonError.new(:SandboxRunOutput,:sandbox => $here, :command=>command_name, :script_location=>command, :arguments => "#{args.join(' ')}") if !File.exists?(command)
    debug2 "running detached #{command} #{args.join(' ')}"
    a=Execute.run("#{command} #{args.join(' ')}", :mode => 'detach')
    debug2 "running detached #{command} ended" 
    [a[:status]]
  end

  def kill
    run_output("kill")
  end

  def status
    begin 
      i,o=run_output("status")
      return i==0
    rescue Exception=> e
      return false
    end
  end
  
  def start_go
    i,o=run_output("start_go")
    raise CotsonError.new(:SandboxStartGo,:command=>"start_go", :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
  end

  def set_mediator_value(y,z)
    i,o=run_output("set_mediator_value",y,z)
    raise CotsonError.new(:SandboxSetMediatorValue,:command=>"set_mediator_value", :key=>y, :value=>z, :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
  end

  def get_mediator_value(x)
    i,o=run_output("get_mediator_value",x)
    raise CotsonError.new(:SandboxGetMediatorValue,:command=>"get_mediator_value", :key=>x, :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
    return o
  end

  def get_vnc_webport
    i,o=run_output("get_vnc_webport")
    raise CotsonError.new(:SandboxGetMediatorValue,:command=>"get_vnc_webport", :key=>x, :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
    return o
  end

  def mediator
    i,o=run_output('mediator')
    raise CotsonError.new(:SandboxMediator,:command=>"mediator", :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
    return o
  end

  def clean(force=false)
    begin
      if force || @opts[:clean_sandbox]
        debug2 "about to clean sandbox #{to_str}"
        run('clean')
      else
        debug2 "sandbox left in #{to_str}"
      end
    rescue
    end
  end
  
  def copy_files(params)
    cmd=""
    cmd+="-d #{params[:destination]} " if params[:destination]
    cmd+="-t #{params[:type]} "        if params[:type]
    cmd+="-f '#{params[:filter]}' "    if params[:filter]
    cmd+="-p #{params[:prefix]} "      if params[:prefix]
    i,o=run_output("copy_files", cmd)
    raise CotsonError.new(:SandboxCopyFiles,:command=>"copy_files #{cmd}", :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
  end

  def list_files(params)
    cmd=""
    cmd += "-t #{params[:type]} "     if params[:type]
    cmd += "-f '#{params[:filter]}' " if params[:filter]
    i,o=run_output("list_files",cmd)
    raise CotsonError.new(:SandboxListFiles,:command=>"list_files #{cmd}", :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
  end

  def consoleX(params)
    cmd=""
    cmd += "-quiet" if params[:quiet] 
    i = run_detached("consoleX",cmd)
    return "#{i}"
  end
  
  def view(params)
    cmd=""
    cmd += "-quiet" if params[:quiet] 
    i = run_detached("view",cmd)
    return "#{i}"
  end

  def stop
    i,o=run_output("stop")
    raise CotsonError.new(:SandboxStop,:command=>"stop", :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
  end
  
  def start
    i,o=run_output("start")
    raise CotsonError.new(:SandboxStart,:command=>"start", :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
  end

  def ngo
    i,o=run_output("ngo")
    raise CotsonError.new(:SandboxNgo,:command=>"ngo", :status=>i, :output=>o.chomp, :location=>"#{@root}") if i != 0
  end
end
