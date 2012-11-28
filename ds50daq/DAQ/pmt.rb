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
    if not executables.keys.include?(host)
      executables[host] = {}
    end
    if not executables[host].keys.include?(program)
      executables[host][program] = {}
    end

    executables[host][program][options] = {"program" => program, 
      "options" => options, "host" => host, "state" => "idle",
      "exitcode" => -1}
  end

  def start()
    mpiThread = Thread.new() do
      configFile = Tempfile.new("config")
      hostsFile = Tempfile.new("hosts")
      begin
        executables.each { |host, hostHash|
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
        Open3.popen3(mpiCmd) { |stdin, stdout, stderr, wait_thr|
          stdout.each { |line|
            parts = line.chomp.split(":")
            if parts.count == 4 and parts[0] == "STARTING"
              executables[parts[1]][parts[2]][parts[3]]["status"] = "running"
              puts "Something is starting."
            elsif parts.count == 5 and parts[0] == "EXITING"
              executables[parts[1]][parts[3]][parts[4]]["status"] = "finished"
              executables[parts[1]][parts[3]][parts[4]]["exitcode"] = parts[2]
              puts "Something is exiting."
            end
          }
        }
      ensure
        configFile.close!
        hostsFile.close!
      end
      
      executables.each { |host, hostHash|
        hostHash.each { |program, programHash|
          programHash.each { |options, optionsHash|
            if optionsHash["status"] != "finished":
                optionsHash["status"] = "interrupted"
            end
            }
          }
        }
    end
  end

  def stop()
    true
  end
end


class PMTRPCHandler
  def initialize(mpiHandler)
    @mpiHandler = mpiHandler
  end

  def status()
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

  def startSystem(configList)
    configList.each do |configItem|
      mpiHandler.addExecutable(configItem["program"], configItem["host"],
                               configItem["port"])
    end

    @mpiHandler.start
    return "success"
  end
end

if __FILE__ == $0
  if ARGV.count != 1 and ARGV.count != 2
    puts "#{$0} <port number> <program definition (optional)>"
    exit
  end

  mpiHandler = MPIHandler.new
  if ARGV.count == 2
    IO.foreach(ARGV[1]) { |definition|
      program, host, port = definition.split(" ")
      mpiHandler.addExecutable(program, host, port)
    }
  end

  rpcHandler = PMTRPCHandler.new(mpiHandler)
  rpcServer = XMLRPC::Server.new(port = ARGV[0])
  rpcServer.add_handler("pmt", rpcHandler)
  rpcServer.serve
end
