import sampleplayer

s = sampleplayer.get("sampleplayer")

def on_pulse():
   step = bespoke.get_step(8) % 16
   #step = 31-step
   start = 148.31
   bpm = 170
   speed = bespoke.get_tempo() / bpm
   bps = bpm/60
   beat = step / 2 - select([5,6,0],step)*16
   #if step % 4 != 0:
   #   beat += random.choice([0,0,0,0,0,0,0,.5,-.5,1])
   #if step == 3:
   #  beat = 0
   sec = start + beat / bps
   play_sample(sec,speed)
   
def select(list,step):
   return list[step % len(list)]   
   
roundrobin = 0
def play_sample(second, speed):
   global roundrobin
   s.set_cue_point(roundrobin, second,1,speed)
   me.play_note(roundrobin,127)
   roundrobin = (roundrobin+1)%8 