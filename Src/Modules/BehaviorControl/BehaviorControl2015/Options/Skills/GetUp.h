/** This option lets the robot stand up when it has fallen down. */
option(GetUp)
{
  /** Determine falldown direction */
  initial_state(lyingDown)
  {
    transition
    {
      if(theFallDownState.state == FallDownState::onGround)
        goto standUp;
    }
    action
    {
      LookForward();
    }
  }

  /** Get up */
  state(standUp)
  {
    transition
    {
      if(action_done)
        goto stand;
    }
    action
    {
      LookForward();
      GetUpEngine();
    }
  }

  /** Try to stand
   *  If the robot still lies down again, try to
   *  stand up again (this changes only anything if the robot is still on the floor after pick up (and nobody sent penalize and game-state is still playing)*/
  state(stand)
  {
    transition
    {
      if(theFallDownState.state == FallDownState::onGround)
        goto standUp;
    }
    action
    {
      LookForward();
      Stand();
    }
  }
}
