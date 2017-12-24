package teamcomm.gui.drawings.common;

import com.jogamp.opengl.GL2;
import data.GameControlData;
import data.Rules;
import data.TeamInfo;
import teamcomm.data.GameState;
import teamcomm.gui.Camera;
import teamcomm.gui.drawings.Static;
import teamcomm.gui.drawings.Text;

/**
 * Drawing for the current GameController info.
 *
 * @author Felix Thielke
 */
public class GameControllerInfo extends Static {

    @Override
    public void draw(final GL2 gl, final Camera camera) {
        final GameControlData data = GameState.getInstance().getLastGameControlData();
        if (data != null) {
            gl.glPushMatrix();
            gl.glTranslatef(0, 4, .5f);
            camera.turnTowardsCamera(gl);

            // Display secondary state
            final String secState;
            switch (data.secGameState) {
                case GameControlData.STATE2_NORMAL:
                    if (data.firstHalf == GameControlData.C_TRUE) {
                        if (data.gameState == GameControlData.STATE_FINISHED) {
                            secState = "Half Time";
                        } else {
                            secState = "First Half";
                        }
                    } else {
                        if (data.gameState == GameControlData.STATE_INITIAL) {
                            secState = "Half Time";
                        } else {
                            secState = "Second Half";
                        }
                    }
                    break;
                case GameControlData.STATE2_OVERTIME:
                    secState = "Overtime";
                    break;
                case GameControlData.STATE2_PENALTYSHOOT:
                    secState = "Penalty Shootout";
                    break;
                case GameControlData.STATE2_TIMEOUT:
                    secState = "Time Out";
                    break;
                default:
                    secState = "";
            }
            Text.drawText(secState, 0, 0.9f, 0.3f);

            int minutes = (data.secsRemaining < 0 ? (-data.secsRemaining) : data.secsRemaining) / 60;
            int seconds = (data.secsRemaining < 0 ? (-data.secsRemaining) : data.secsRemaining) % 60;
            Text.drawText((data.secsRemaining < 0 ? "-" : "") + (minutes < 10 ? "0" : "") + minutes + ":" + (seconds < 10 ? "0" : "") + seconds, 0, 0.6f, 0.3f);

            // Display game state
            final String state;
            switch (data.gameState) {
                case GameControlData.STATE_INITIAL:
                    state = "Initial";
                    break;
                case GameControlData.STATE_READY:
                    state = "Ready";
                    break;
                case GameControlData.STATE_SET:
                    state = "Set";
                    break;
                case GameControlData.STATE_PLAYING:
                    state = "Playing";
                    break;
                case GameControlData.STATE_FINISHED:
                    state = "Finished";
                    break;
                default:
                    state = "";
                    break;
            }
            Text.drawText(state, 0, 0.3f, 0.3f);

            // Display scores
            final TeamInfo teamLeft = data.team[GameState.getInstance().isMirrored() ? 1 : 0];
            final TeamInfo teamRight = data.team[GameState.getInstance().isMirrored() ? 0 : 1];
            Text.drawText("" + teamLeft.score, -0.3f, 0, 0.3f, getColor(teamLeft.teamColor));
            Text.drawText(":", 0, 0, 0.3f);
            Text.drawText("" + teamRight.score, 0.3f, 0, 0.3f, getColor(teamRight.teamColor));
            gl.glPopMatrix();
        }
    }

    private static float[] getColor(final byte color) {
        if (color < 0 || color >= Rules.league.teamColor.length) {
            return new float[]{1, 1, 1, 1};
        }
        return Rules.league.teamColor[color].getComponents(new float[4]);
    }

    @Override
    public boolean hasAlpha() {
        return false;
    }

    @Override
    public int getPriority() {
        return 0;
    }

}
