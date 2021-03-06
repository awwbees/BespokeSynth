import grid
g = grid.get("grid")

def on_pulse():
   chance = this.get('a')
   this.output(chance)
   for pitch in range(6):
      sequence = []
      for step in range(16):
         if g.get(step, pitch) > 0:
            shift = 0
            if step > 0 and step % 2 == 0 and random.random() < chance:
               shift = -1
            time = (step+shift)/16.0
            this.schedule_note(time, pitch, 100, 1.0/16)
            g.highlight_cell(step+shift, pitch, time, 1.0/8) 