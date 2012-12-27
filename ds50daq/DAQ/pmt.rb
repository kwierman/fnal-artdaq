#!/usr/bin/env ruby

require "xmlrpc/server"
require "open3"
require "tempfile"

class MPIHandler
  def executables
    @executables
  end

  def initialize()
    @mpiThread = nil
    @executables = {}
  end

  def addExecutable(program, host, options)
    # Add an executable to be managed by PMT.  The executable definitions are
    # kept in a three level hash to they can more easily be found.
    if not @executables.keys.include?(host)
      @executables[host] = {}
    end
    if not @executables[host].keys.include?(program)
      @executables[host][program] = {}
    end

    @executables[host][program][options] = {"program" => program, 
      "options" => options, "host" => host, "state" => "idle",
      "exitcode" => -1}
  end

  def start()
    # Use mpirun_rsh to launch all of the configured applications.  This will
    # create temporary config files and pass those to mpirun_rsh.  The output
    # will be monitored for prompts from the wrapper script and will use those
    # to update the state of the application in the executables member data.
    #
    # Note that this will immediately spawn a new thread to handle monitoring
    # MPI and return.
    @executables.each { |host, hostHash|
      hostHash.each { |program, programHash|
        programHash.each { |options, optionsHash|
          if optionsHash["state"] != "idle":
              return
          end
        }
      }
    }

    mpiThread = Thread.new() do
      configFile = Tempfile.new("config")
      hostsFile = Tempfile.new("hosts")
      begin
        @executables.each { |host, hostHash|
          hostHash.each { |program, programHash|
            programHash.each { |options, optionsHash|
              configFile.write("-n 1 : mpi_wrapper.sh ")
              configFile.write(program + " ")
              configFile.write(options + "\n")
              hostsFile.write(host + "\n")
            }
          }
        }
        
        mpiCmd = "mpirun_rsh -ssh -config %s -hostfile %s" % [configFile.path,
                                                              hostsFile.path]
        configFile.rewind
        hostsFile.rewind
        Open3.popen3(mpiCmd) { |stdin, stdout, stderr|
          stdout.each { |line|
            parts = line.chomp.split(":")
            if parts.count == 4 and parts[0] == "STARTING"
              @executables[parts[1]][parts[2]][parts[3]]["state"] = "running"
              puts "Something is starting."
            elsif parts.count == 5 and parts[0] == "EXITING"
              @executables[parts[1]][parts[3]][parts[4]]["state"] = "finished"
              @executables[parts[1]][parts[3]][parts[4]]["exitcode"] = parts[2]
              puts "Something is exiting."
            end
          }
        }
      ensure
        configFile.close!
        hostsFile.close!
      end
      
      # If we got here and find applications that are not in the finished state
      # it more than likely means that mpirun_rsh exited and we don't really
      # know what is happening with our MPI application.
      @executables.each { |host, hostHash|
        hostHash.each { |program, programHash|
          programHash.each { |options, optionsHash|
            if optionsHash["state"] != "finished":
                optionsHash["state"] = "interrupted"
            end
            }
          }
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
      @executables.each { |host, hostHash|
        hostHash.each { |program, programHash|
          programHash.each { |options, optionsHash|
            configFile.write("-n 1 : mpi_cleaner.sh ")
            configFile.write(program + " ")
            configFile.write(options + "\n")
            hostsFile.write(host + "\n")
          }
        }
      }
      
      mpiCmd = "mpirun_rsh -ssh -config %s -hostfile %s" % [configFile.path,
                                                            hostsFile.path]
      configFile.rewind
      hostsFile.rewind
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
    @mpiHandler.executables.each { |host, hostHash|
      hostHash.each { |program, programHash|
        programHash.each { |options, optionsHash|
          statusItems << optionsHash
        }
      }
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
    if argv.count != 1 and argv.count != 2
      puts "#{$0} <port number> <program definition (optional)>"
      exit
    end

    @mpiHandler = MPIHandler.new
    if argv.count == 2
      IO.foreach(argv[1]) { |definition|
        program, host, port = definition.split(" ")
        @mpiHandler.addExecutable(program, host, port)
      }
    end
    
    # Instantiate the RPM handler and then create the RPC server.
    @rpcHandler = PMTRPCHandler.new(@mpiHandler)
    @rpcServer = XMLRPC::Server.new(port = argv[0])
    @rpcServer.add_handler("pmt", @rpcHandler)

    # If we've been passed our executable list via the command line we need
    # to spawn off the MPI processes right away.
    if @mpiHandler.executables.size > 0:
        @mpiHandler.start
    end
  end

  def start
    # Startup the RPC server.  This does not return.
    @rpcServer.serve    
    end

  def stop
    # Shutdown the RPC server and prompt the MPI handler class to attempt
    # to clean up.
    @rpcServer.shutdown
    @mpiHandler.stop
  end
end

if __FILE__ == $0
  pmt = PMT.new(ARGV)
  pmt.start
end
