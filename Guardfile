guard :shell do
  watch %r{\.(def\.gen)$} do | m |
    $gq.go "make clean"
    $qq.go "make test"
  end
  watch %r{\.(c|h|def)*$} do | m |
    $gq.go "make test"
  end
  watch %r{Makefile$} do | m |
    $gq.go "make clean"
    $gq.go "make test"
  end
end

require 'thread'
require 'term/ansicolor'

class GuardQueue
  C = Term::ANSIColor
  def initialize
    @stage = [ ]
    @queue = Queue.new
  end

  def msg str
    $stderr.puts "\n  GQ #{$$} #{str}"
  end

  def notice str; msg C.yellow(str); end
  def error  str; msg C.red(str); end
  def ok     str; msg C.green(str); end

  def go cmd
    if @stage.include? cmd
      # msg "ALREADY  #{cmd.inspect}"
    else
      @stage.unshift cmd
      # notice "STAGE   #{@stage.inspect}"
    end
    if @working
      notice "WORKING #{@working.inspect}" unless @working == cmd
    else
      while cmd = @stage.pop
        notice "QUEUEING #{cmd.inspect}"
        @queue.enq cmd
      end
    end
  end

  def start!
    at_exit do
      @stopping = true
      go :stop
      @thread.join
    end

    @thread = Thread.new do
      notice "WAITING"
      while true
        break if @stopping
        case cmd = @queue.deq
        when :stop
          break
        else
          break if @stopping
          begin
            @working = cmd
            notice "RUNNING #{cmd.inspect}"
            if system(cmd)
              ok "  OK #{cmd.inspect}"
            else
              error "  FAILED #{cmd.inspect}"
            end
          ensure
            sleep 0.5
            # notice "   DONE #{cmd.inspect}"
            @working = nil
          end
        end
      end
      notice "STOPPING"
    end
    self
  end
end

$gq ||= GuardQueue.new.start!

