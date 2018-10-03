package com.example.garyrendle.mis_cpp_test;

public class RoadInfo {
    private int maxSpeed;
    private  String roadName;

    public RoadInfo(int maxSpeed, String roadName) {
            this.maxSpeed = maxSpeed;
            this.roadName = roadName;
    }

    public int getMaxSpeed() {
        return maxSpeed;
    }

    public String getRoadName() {
        return roadName;
    }

    public void setMaxSpeed(int maxSpeed) {
        this.maxSpeed = maxSpeed;
    }

    public void setRoadName(String roadName) {
        this.roadName = roadName;
    }
}
