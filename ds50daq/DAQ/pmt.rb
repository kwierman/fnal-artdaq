#!/usr/bin/env ruby

require "xmlrpc/server"
require "open3"
require "tempfile"
require "logger"

class MPIHandler
  def executables
    @executables
  end

  def createLogger()
    # Create a logger object.  This will create log files with the name:
    #  pmt-yearmondate-hourminsec.log
    # Where the timestamp is the time that executables were spawned.
    currentTime = Time.now
    logFileName = "pmt-%d%02d%02d-%02d%02d%02d.log" % [ currentTime.year, 
                                                        currentTime.month,
                                                        currentTime.day,
                                                        currentTime.hour,
                                                        currentTime.min,
                                                        currentTime.sec ]
    @logger = Logger.new(logFileName, 5, 10240000)
    @logger.level = Logger::INFO
    @logger.formatter = proc do |severity, datetime, progname, msg|
      "#{datetime}: #{msg}\n"
    end
    puts "Log file basename: %s" % logFileName
  end

  def initialize()
    @mpiThread = nil
    @executables = []
    self.createLogger
  end

  def addExecutable(program, host, options)
    # Add an executable to be managed by PMT.  The executable definitions are
    # kept in a list ordered by MPI rank.  The first process added will receive
    # rank 0.
    @executables << {"program" => program, "options" => options, "host" => host,
      "state" => "idle", "exitcode" => -1}
  end

  def buildMPICommand(configFileHandle, hostsFileHandle, wrapperScript)
    @executables.each { |optionsHash|
      configFileHandle.write("-n 1 : %s " % [wrapperScript])
      configFileHandle.write(optionsHash["program"] + " ")
      configFileHandle.write(optionsHash["options"] + "\n")
      hostsFileHandle.write(optionsHash["host"] + "\n")
    }
        
    disableCpuAffinity = "export MV2_ENABLE_AFFINITY=0;"
    mpiCmd = "mpirun_rsh -rsh -config %s -hostfile %s" % [configFileHandle.path,
                                                          hostsFileHandle.path]
    configFileHandle.rewind
    hostsFileHandle.rewind
    return mpiCmd
  end

  def parseOutput(line)
    parts = line.chomp.split(":")
    if parts.count == 4 and parts[0] == "STARTING"
      @executables.each { |optionsHash|
        if parts[2] == optionsHash["program"] and parts[1] == optionsHash["host"] and 
            parts[3] == optionsHash["options"]
          optionsHash["state"] = "running"
          puts "%s on %s is starting." % [parts[2], parts[1]]
          break
        end
      }
    elsif parts.count == 5 and parts[0] == "EXITING"
      @executables.each { |optionsHash|
        if parts[2] == optionsHash["program"] and parts[1] == optionsHash["host"] and
            parts[3] == optionsHash["options"]
          optionsHash["state"] = "finished"
          optionsHash["exitcode"] = parts[2]
          puts "%s on %s is exiting." % [parts[2], parts[1]]
          break
        end
      }
    end
  end

  def handleIO(stdoutHandle, stderrHandle)
    stdBuffers = {}
    while true
      readyFDs = IO.select([stdoutHandle, stderrHandle])
      readyReadFDs = readyFDs[0]
      readyReadFDs.each do |fd|
        if not stdBuffers.keys.include?(fd)
          stdBuffers[fd] = fd.read(1)
        else
          stdBuffers[fd] += fd.read(1)
        end
        
        if stdBuffers[fd].count("\n") != 0
          lines = stdBuffers[fd].split("\n")
          if stdBuffers[fd][-1].chr == "\n"
            stdBuffers[fd] = ""
          else
            stdBuffers[fd] = lines.pop
          end
          
          lines.each { |line|
            @logger.info(line.chomp)
            self.parseOutput(line)
          }
        end
      end
    end
  end

  def start()
    # Use mpirun_rsh to launch all of the configured applications.  This will
    # create temporary config files and pass those to mpirun_rsh.  The output
    # will be monitored for prompts from the wrapper script and will use those
    # to update the state of the application in the executables member data.
    #
    # Note that this will immediately spawn a new thread to handle monitoring
    # MPI and return.
    #
    # First step is to iterate through all of the configured executables and
    # make sure that everything is idle and nothing has been started already.
    @executables.each { |optionsHash|
      if optionsHash["state"] != "idle":
          return
      end
    }

    mpiThread = Thread.new() do
      configFile = Tempfile.new("config")
      hostsFile = Tempfile.new("hosts")
      begin
        mpiCmd = self.buildMPICommand(configFile, hostsFile, "mpi_wrapper.sh")
        Open3.popen3(mpiCmd) { |stdin, stdout, stderr|
          self.handleIO(stdout, stderr)
        }
      ensure
        configFile.close!
        hostsFile.close!
      end
      
      # If we got here and find applications that are not in the finished state
      # it more than likely means that mpirun_rsh exited and we don't really
      # know what is happening with our MPI application.
      @executables.each { |optionsHash|
        if optionsHash["state"] != "finished":
            optionsHash["state"] = "interrupted"
        end
        }
    end
  end

  def stop
    # I have yet to find a good way to clean up.  Killing off the mpirun_rsh
    # process does nothing to the child processes.  For the time being we'll use
    # the sledge hammer approach and use MPI to submit jobs to kill off everything
    # that we spawned.
    configFile = Tempfile.new("config")
    hostsFile = Tempfile.new("hosts")
    begin
      mpiCmd = self.buildMPICommand(configFile, hostsFile, "mpi_cleaner.sh")

      Open3.popen3(mpiCmd) { |stdin, stdout, stderr|
        # Block until this is done.
        stdout.each { |line|
          puts line
        }
      }

      # This isn't the best but after stdout is closed it seems MPI still needs
      # some time to clean everything up.
      sleep(1)
    ensure
      configFile.close!
      hostsFile.close!
    end
  end
end

class PMTRPCHandler
  def initialize(mpiHandler)
    @mpiHandler = mpiHandler
  end

  def alive
    # Trivial method used by the unit test to verify that PMT is reachable via
    # XMLRPC.
    return true
  end

  def status
    # Iterate through all of the executables currently configured and generate
    # a list that can be returned to the caller.
    statusItems = []
    @mpiHandler.executables.each { |optionsHash|
      statusItems << optionsHash
    }

    return statusItems
  end

  def addExecutables(configList)
    # Add one or more applications to the list of applications that PMT is 
    # managing. Note that this must be done before startSystem is called. 
    configList.each do |configItem|
      @mpiHandler.addExecutable(configItem["program"], configItem["host"],
                                configItem["port"])
    end
  end

  def startSystem
    # Have the MPI handler code launch any applications that have been
    # configured.
    @mpiHandler.start
    return true
  end
end

class PMT
  def initialize(argv)
    # Verify that the command line options are reasonable.  If the user has
    # passed in a config file parse that and load the specified executables
    # into the MPI handler class.
    if argv.count != 2 and argv.count != 3
      puts "#{$0} -p <port number> <program definition (optional)>"
      exit
    end

    @rpcThread = nil

    @mpiHandler = MPIHandler.new
    if argv.count == 3
      IO.foreach(argv[2]) { |definition|
        program, host, port = definition.split(" ")
        @mpiHandler.addExecutable(program, host, port)
      }
    end
    
    # Instantiate the RPM handler and then create the RPC server.
    @rpcHandler = PMTRPCHandler.new(@mpiHandler)
    @rpcServer = XMLRPC::Server.new(port = argv[1])
    @rpcServer.add_handler("pmt", @rpcHandler)

    # If we've been passed our executable list via the command line we need
    # to spawn off the MPI processes right away.
    if @mpiHandler.executables.size > 0:
        @mpiHandler.start
    end
  end

  def start
    # Startup the RPC server.  This does not return.
    @rpcThread = Thread.new() do
      @rpcServer.serve
    end
  end

  def stop
    # Shutdown the RPC server and prompt the MPI handler class to attempt
    # to clean up.
    @rpcServer.shutdown
    if @rpcThread != nil
      @rpcThread.exit
    end
    @mpiHandler.stop
  end
end

if __FILE__ == $0
  pmt = PMT.new(ARGV)
  pmt.start

  signals = %w[INT TERM HUP] & Signal.list.keys
  signals.each { 
    |signal| trap(signal) { 
      puts "Cleaning up..."
      pmt.stop 
      exit
    } 
  }

  while true
    sleep(1)
  end
end
