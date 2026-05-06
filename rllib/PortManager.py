import ray

@ray.remote
class PortManagerActor:
    def __init__(self, start=5100, end=6000):
        self.current = start
        self.end = end

    def get_next_port(self):
        port = self.current
        if self.current >= self.end - 1:
            self.current = 5100
        else:
            self.current += 1
        return port
