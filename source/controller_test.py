import pygame
import sys

pygame.init()
pygame.joystick.init()

joystick_count = pygame.joystick.get_count()
if joystick_count == 0:
    print("No joystick detected!")
    sys.exit()
else:
    # Initialize the first joystick
    joystick = pygame.joystick.Joystick(0)
    joystick.init()

print("Press any button on the joystick")

# Main loop
running = True
while running:
    for event in pygame.event.get():
        if event.type == pygame.QUIT:
            running = False
        elif event.type == pygame.JOYBUTTONDOWN:
            print(f"Button {event.button} pressed")
        elif event.type == pygame.JOYBUTTONUP:
            print(f"Button {event.button} released")

pygame.quit()
